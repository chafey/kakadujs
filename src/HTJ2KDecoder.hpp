// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <memory>
#include <limits.h>

// Kakadu core includes
#include "kdu_elementary.h"
#include "kdu_messaging.h"
#include "kdu_params.h"
#include "kdu_compressed.h"
#include "kdu_sample_processing.h"
#include "kdu_utils.h" // Access `kdu_memsafe_mul' etc. for safe mem calcs

#include "kdu_stripe_decompressor.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

#include "FrameInfo.hpp"
#include "Point.hpp"
#include "Size.hpp"

#define ojph_div_ceil(a, b) (((a) + (b)-1) / (b))

/// <summary>
/// JavaScript API for decoding HTJ2K bistreams with OpenJPH
/// </summary>
class HTJ2KDecoder
{
public:
  /// <summary>
  /// Constructor for decoding a HTJ2K image from JavaScript.
  /// </summary>
  HTJ2KDecoder()
  {
  }

#ifdef __EMSCRIPTEN__
  /// <summary>
  /// Resizes encoded buffer and returns a TypedArray of the buffer allocated
  /// in WASM memory space that will hold the HTJ2K encoded bitstream.
  /// JavaScript code needs to copy the HTJ2K encoded bistream into the
  /// returned TypedArray.  This copy operation is needed because WASM runs
  /// in a sandbox and cannot access memory managed by JavaScript.
  /// </summary>
  emscripten::val getEncodedBuffer(size_t encodedSize)
  {
    encoded_.resize(encodedSize);
    return emscripten::val(emscripten::typed_memory_view(encoded_.size(), encoded_.data()));
  }

  /// <summary>
  /// Returns a TypedArray of the buffer allocated in WASM memory space that
  /// holds the decoded pixel data
  /// </summary>
  emscripten::val getDecodedBuffer()
  {
    return emscripten::val(emscripten::typed_memory_view(decoded_.size(), decoded_.data()));
  }
#else
  /// <summary>
  /// Returns the buffer to store the encoded bytes.  This method is not exported
  /// to JavaScript, it is intended to be called by C++ code
  /// </summary>
  std::vector<uint8_t> &getEncodedBytes()
  {
    return encoded_;
  }

  /// <summary>
  /// Returns the buffer to store the decoded bytes.  This method is not exported
  /// to JavaScript, it is intended to be called by C++ code
  /// </summary>
  const std::vector<uint8_t> &getDecodedBytes() const
  {
    return decoded_;
  }
#endif

  /// <summary>
  /// Reads the header from an encoded HTJ2K bitstream.  The caller must have
  /// copied the HTJ2K encoded bitstream into the encoded buffer before
  /// calling this method, see getEncodedBuffer() and getEncodedBytes() above.
  /// </summary>
  void readHeader()
  {
    kdu_core::kdu_compressed_source_buffered input(encoded_.data(), encoded_.size());
    kdu_core::kdu_codestream codestream;
    codestream.create(&input);
    codestream.set_fussy(); // Set the parsing error tolerance.

    // Determine number of components to decompress -- simple app only writes PNM
    kdu_core::kdu_dims dims;
    codestream.get_dims(0, dims);

    int num_components = codestream.get_num_components();
    if (num_components == 2)
      num_components = 1;
    else if (num_components >= 3)
    { // Check that components have consistent dimensions (for PPM file)
      num_components = 3;
      kdu_core::kdu_dims dims1;
      codestream.get_dims(1, dims1);
      kdu_core::kdu_dims dims2;
      codestream.get_dims(2, dims2);
      if ((dims1 != dims) || (dims2 != dims))
        num_components = 1;
    }
    codestream.apply_input_restrictions(0, num_components, 0, 0, NULL);
    frameInfo_.width = dims.size.x;
    frameInfo_.height = dims.size.y;
    frameInfo_.componentCount = num_components;
    frameInfo_.bitsPerSample = codestream.get_bit_depth(0);
    frameInfo_.isSigned = codestream.get_signed(0);
    codestream.destroy();
    input.close(); // Not really necessary here.
  }

  /// <summary>
  /// Calculates the resolution for a given decomposition level based on the
  /// current values in FrameInfo (which is populated via readHeader() and
  /// decode()).  level = 0 = full res, level = _numDecompositions = lowest resolution
  /// </summary>
  Size calculateSizeAtDecompositionLevel(int decompositionLevel)
  {
    Size result(frameInfo_.width, frameInfo_.height);
    while (decompositionLevel > 0)
    {
      result.width = ojph_div_ceil(result.width, 2);
      result.height = ojph_div_ceil(result.height, 2);
      decompositionLevel--;
    }
    return result;
  }

  /// <summary>
  /// Decodes the encoded HTJ2K bitstream.  The caller must have copied the
  /// HTJ2K encoded bitstream into the encoded buffer before calling this
  /// method, see getEncodedBuffer() and getEncodedBytes() above.
  /// </summary>
  void decode()
  {
    decode_(0);
  }

  /// <summary>
  /// Decodes the encoded HTJ2K bitstream to the requested decomposition level.
  /// The caller must have copied the HTJ2K encoded bitstream into the encoded
  /// buffer before calling this method, see getEncodedBuffer() and
  ///  getEncodedBytes() above.
  /// </summary>
  void decodeSubResolution(size_t decompositionLevel)
  {
    decode_(decompositionLevel);
  }

  /// <summary>
  /// returns the FrameInfo object for the decoded image.
  /// </summary>
  const FrameInfo &getFrameInfo() const
  {
    return frameInfo_;
  }

  /// <summary>
  /// returns the number of wavelet decompositions.
  /// </summary>
  const size_t getNumDecompositions() const
  {
    return numDecompositions_;
  }

  /// <summary>
  /// returns true if the image is lossless, false if lossy
  /// </summary>
  const bool getIsReversible() const
  {
    return isReversible_;
  }

  /// <summary>
  /// returns progression order.
  // 0 = LRCP
  // 1 = RLCP
  // 2 = RPCL
  // 3 = PCRL
  // 4 = CPRL
  /// </summary>
  const size_t getProgressionOrder() const
  {
    return progressionOrder_;
  }

  /// <summary>
  /// returns the down sampling used for component.
  /// </summary>
  Point getDownSample(size_t component) const
  {
    return downSamples_[component];
  }

  /// <summary>
  /// returns the image offset
  /// </summary>
  Point getImageOffset() const
  {
    return imageOffset_;
  }

  /// <summary>
  /// returns the tile size
  /// </summary>
  Size getTileSize() const
  {
    return tileSize_;
  }

  /// <summary>
  /// returns the tile offset
  /// </summary>
  Point getTileOffset() const
  {
    return tileOffset_;
  }

  /// <summary>
  /// returns the block dimensions
  /// </summary>
  Size getBlockDimensions() const
  {
    return blockDimensions_;
  }

  /// <summary>
  /// returns the precinct for the specified resolution decomposition level
  /// </summary>
  Size getPrecinct(size_t level) const
  {
    return precincts_[level];
  }

  /// <summary>
  /// returns the number of layers
  /// </summary>
  int32_t getNumLayers() const
  {
    return numLayers_;
  }

  /// <summary>
  /// returns whether or not a color transform is used
  /// </summary>
  bool getIsUsingColorTransform() const
  {
    return isUsingColorTransform_;
  }

private:
  void decode_(size_t decompositionLevel)
  {
    kdu_core::kdu_compressed_source_buffered input(encoded_.data(), encoded_.size());
    kdu_core::kdu_codestream codestream;
    codestream.create(&input);
    codestream.set_fussy(); // Set the parsing error tolerance.

    // Determine number of components to decompress
    kdu_core::kdu_dims dims;
    codestream.get_dims(0, dims);

    int num_components = codestream.get_num_components();
    if (num_components == 2)
      num_components = 1;
    else if (num_components >= 3)
    { // Check that components have consistent dimensions (for PPM file)
      num_components = 3;
      kdu_core::kdu_dims dims1;
      codestream.get_dims(1, dims1);
      kdu_core::kdu_dims dims2;
      codestream.get_dims(2, dims2);
      if ((dims1 != dims) || (dims2 != dims))
        num_components = 1;
    }
    codestream.apply_input_restrictions(0, num_components, 0, 0, NULL);
    frameInfo_.width = dims.size.x;
    frameInfo_.height = dims.size.y;
    frameInfo_.componentCount = num_components;
    frameInfo_.bitsPerSample = codestream.get_bit_depth(0);
    frameInfo_.isSigned = codestream.get_signed(0);

    size_t bytesPerPixel = (frameInfo_.bitsPerSample + 1) / 8;

    // Now decompress the image in one hit, using `kdu_stripe_decompressor'
    size_t num_samples = kdu_core::kdu_memsafe_mul(num_components,
                                                   kdu_core::kdu_memsafe_mul(dims.size.x,
                                                                             dims.size.y));
    decoded_.resize(num_samples * bytesPerPixel);

    kdu_core::kdu_byte *buffer = decoded_.data();
    kdu_supp::kdu_stripe_decompressor decompressor;
    decompressor.start(codestream);
    int stripe_heights[3] = {dims.size.y, dims.size.y, dims.size.y};
    decompressor.pull_stripe((kdu_core::kdu_int16 *)buffer, stripe_heights);
    decompressor.finish();

    // Write image buffer to file and clean up
    codestream.destroy();
    input.close(); // Not really necessary here.
  }

  std::vector<uint8_t> encoded_;
  std::vector<uint8_t> decoded_;
  FrameInfo frameInfo_;
  std::vector<Point> downSamples_;
  size_t numDecompositions_;
  bool isReversible_;
  size_t progressionOrder_;
  Point imageOffset_;
  Size tileSize_;
  Point tileOffset_;
  Size blockDimensions_;
  std::vector<Size> precincts_;
  int32_t numLayers_;
  bool isUsingColorTransform_;
};
