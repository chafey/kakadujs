// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <memory>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
// Kakadu core includes
#include "kdu_elementary.h"
#include "kdu_messaging.h"
#include "kdu_params.h"
#include "kdu_compressed.h"
#include "kdu_sample_processing.h"
#include "kdu_utils.h" // Access `kdu_memsafe_mul' etc. for safe mem calcs
#include "jp2.h" 
// Application level includes
#include "kdu_stripe_compressor.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

#include "FrameInfo.hpp"


class kdu_buffer_target : public kdu_core::kdu_compressed_target {
    public: // Member functions
    kdu_buffer_target(std::vector<uint8_t>& encoded) :encoded_(encoded) {
      encoded_.resize(0);
    }
    ~kdu_buffer_target() { return; } // Destructor must be virtual
    int get_capabilities() { return KDU_TARGET_CAP_CACHED; }
    bool write(const kdu_core::kdu_byte *buf, int num_bytes) { 
      printf("num_bytes=%d\n", num_bytes);
      const size_t size = encoded_.size();
      printf("encoded.size()=%zu\n", size);
      encoded_.resize(size + num_bytes);

      memcpy(encoded_.data() + size, buf, num_bytes);
      return true; 
    }
  private: // Data
    std::vector<uint8_t>& encoded_;
  };

/// <summary>
/// JavaScript API for encoding images to HTJ2K bitstreams with OpenJPH
/// </summary>
class HTJ2KEncoder
{
public:
  /// <summary>
  /// Constructor for encoding a HTJ2K image from JavaScript.
  /// </summary>
  HTJ2KEncoder() : decompositions_(5),
                   lossless_(true),
                   quantizationStep_(-1.0),
                   progressionOrder_(2), // RPCL
                   blockDimensions_(64, 64)
  {
  }

#ifdef __EMSCRIPTEN__
  /// <summary>
  /// Resizes the decoded buffer to accomodate the specified frameInfo.
  /// Returns a TypedArray of the buffer allocated in WASM memory space that
  /// will hold the pixel data to be encoded.  JavaScript code needs
  /// to copy the pixel data into the returned TypedArray.  This copy
  /// operation is needed because WASM runs in a sandbox and cannot access
  /// data managed by JavaScript
  /// </summary>
  /// <param name="frameInfo">FrameInfo that describes the pixel data to be encoded</param>
  /// <returns>
  /// TypedArray for the buffer allocated in WASM memory space for the
  /// source pixel data to be encoded.
  /// </returns>
  emscripten::val getDecodedBuffer(const FrameInfo &frameInfo)
  {
    frameInfo_ = frameInfo;
    const size_t bytesPerPixel = (frameInfo_.bitsPerSample + 8 - 1) / 8;
    const size_t decodedSize = frameInfo_.width * frameInfo_.height * frameInfo_.componentCount * bytesPerPixel;
    printf("decodedSize=%lu\n", decodedSize)
    printf("bytesPerPixel=%lu\n", bytesPerPixel)
    downSamples_.resize(frameInfo_.componentCount);
    for (int c = 0; c < frameInfo_.componentCount; ++c)
    {
      downSamples_[c].x = 1;
      downSamples_[c].y = 1;
    }

    decoded_.resize(decodedSize);
    return emscripten::val(emscripten::typed_memory_view(decoded_.size(), decoded_.data()));
  }

  /// <summary>
  /// Returns a TypedArray of the buffer allocated in WASM memory space that
  /// holds the encoded pixel data.
  /// </summary>
  /// <returns>
  /// TypedArray for the buffer allocated in WASM memory space for the
  /// encoded pixel data.
  /// </returns>
  emscripten::val getEncodedBuffer()
  {
    return emscripten::val(emscripten::typed_memory_view(encoded_.size(), encoded_.data()));
  }
#else
  /// <summary>
  /// Returns the buffer to store the decoded bytes.  This method is not
  /// exported to JavaScript, it is intended to be called by C++ code
  /// </summary>
  std::vector<uint8_t> &getDecodedBytes(const FrameInfo &frameInfo)
  {
    frameInfo_ = frameInfo;
    downSamples_.resize(frameInfo_.componentCount);
    for (int c = 0; c < frameInfo_.componentCount; ++c)
    {
      downSamples_[c].x = 1;
      downSamples_[c].y = 1;
    }
    return decoded_;
  }

  /// <summary>
  /// Returns the buffer to store the encoded bytes.  This method is not
  /// exported to JavaScript, it is intended to be called by C++ code
  /// </summary>
  const std::vector<uint8_t> &getEncodedBytes() const
  {
    return encoded_;
  }
#endif

  /// <summary>
  /// Sets the number of wavelet decompositions and clears any precincts
  /// </summary>
  void setDecompositions(size_t decompositions)
  {
    decompositions_ = decompositions;
    precincts_.resize(0);
  }

  /// <summary>
  /// Sets the quality level for the image.  If lossless is false then
  /// quantizationStep controls the lossy quantization applied.  quantizationStep
  /// is ignored if lossless is true
  /// </summary>
  void setQuality(bool lossless, float quantizationStep)
  {
    lossless_ = lossless;
    quantizationStep_ = quantizationStep;
  }

  /// <summary>
  /// Sets the progression order
  /// 0 = LRCP
  /// 1 = RLCP
  /// 2 = RPCL
  /// 3 = PCRL
  /// 4 = CPRL
  /// </summary>
  void setProgressionOrder(size_t progressionOrder)
  {
    progressionOrder_ = progressionOrder;
  }

  /// <summary>
  /// Sets the down sampling for component
  /// </summary>
  void setDownSample(size_t component, Point downSample)
  {
    downSamples_[component] = downSample;
  }

  /// <summary>
  /// Sets the image offset
  /// </summary>
  void setImageOffset(Point imageOffset)
  {
    imageOffset_ = imageOffset;
  }

  /// <summary>
  /// Sets the tile size
  /// </summary>
  void setTileSize(Size tileSize)
  {
    tileSize_ = tileSize;
  }

  /// <summary>
  /// Sets the tile offset
  /// </summary>
  void setTileOffset(Point tileOffset)
  {
    tileOffset_ = tileOffset;
  }

  /// <summary>
  /// Sets the block dimensions
  /// </summary>
  void setBlockDimensions(Size blockDimensions)
  {
    blockDimensions_ = blockDimensions;
  }

  /// <summary>
  /// Sets the number of precincts
  /// </summary>
  void setNumPrecincts(size_t numLevels)
  {
    precincts_.resize(numLevels);
  }

  /// <summary>
  /// Sets the precinct for the specified level.  You must
  /// call setNumPrecincts with the number of levels first
  /// </summary>
  void setPrecinct(size_t level, Size precinct)
  {
    precincts_[level] = precinct;
  }

  /// <summary>
  /// Sets whether or not the color transform is used
  /// </summary>
  void setIsUsingColorTransform(bool isUsingColorTransform)
  {
    isUsingColorTransform_ = isUsingColorTransform;
  }

  /// <summary>
  /// Executes an HTJ2K encode using the data in the source buffer.  The
  /// JavaScript code must copy the source image frame into the source
  /// buffer before calling this method.  See documentation on getSourceBytes()
  /// above
  /// </summary>
  void encode()
  {
    //int num_components=0, height, width;
    printf("decoded_.size()=%lu\n", decoded_.size());
    // Construct code-stream object
    kdu_core::siz_params siz;
    siz.set(Scomponents,0,0,frameInfo_.componentCount);
    siz.set(Sdims,0,0,frameInfo_.height);  // Height of first image component
    siz.set(Sdims,0,1,frameInfo_.width);   // Width of first image component
    siz.set(Sprecision,0,0,frameInfo_.bitsPerSample);  // Image samples have original bit-depth of 8
    siz.set(Ssigned,0,0,frameInfo_.isSigned); // Image samples are originally unsigned
    kdu_core::kdu_params *siz_ref = &siz; siz_ref->finalize();


      // Finalizing the siz parameter object will fill in the myriad SIZ
      // parameters we have not explicitly specified in this simple example.
      // The capabilities of the finalization process are documented in
      // "kdu_params.h".  Note that we execute the virtual member function
      // through a pointer, since the address of the function is not explicitly
      // exported by the core DLL (minimizes the export table).
    kdu_buffer_target target(encoded_); // Can also use `kdu_simple_file_target'
    kdu_supp::jp2_family_tgt tgt;
    tgt.open(&target);
    kdu_supp::jp2_target output;
    output.open(&tgt);
    kdu_supp::jp2_dimensions dims = output.access_dimensions(); dims.init(&siz);
    kdu_supp::jp2_colour colr = output.access_colour();
    //colr.init( (frameInfo_.componentCount==3) ? kdu_supp::JP2_sRGB_SPACE : kdu_supp::JP2_bilevel1_SPACE);
    colr.init(kdu_supp::JP2_sLUM_SPACE);
    output.write_header();
    output.open_codestream(true);

      // As an alternative to raw code-stream output, you may wish to wrap
      // the code-stream in a JP2/JPH file, which then allows you to add a
      // additional information concerning the colour and geometric properties
      // of the image, all of which should be respected by conforming readers.
      // To do this, include "jp2.h" and declare "output" to be of class
      // `jp2_target' instead of `kdu_simple_file_target' or
      // `kdu_platform_file_target'.  Rather than opening the `jp2_target'
      // object directly, you first create a `jp2_family_tgt' object, using
      // it to open the file to which you want to write, and then pass it to
      // `jp2_target::open'.  At a minimum, you must execute the following
      // additional configuration steps to create a JP2 file:
      //     jp2_dimensions dims = output.access_dimensions(); dims.init(&siz);
      //     jp2_colour colr = output.access_colour();
      //     colr.init((num_components==3)?:JP2_sRGB_SPACE:JP2_sLUM_SPACE);
      // Of course, there is a lot more you can do with JP2/JPH files.  Read the
      // interface descriptions in "jp2.h" and/or take a look at the more
      // sophisticated demonstration in "kdu_compress.cpp".  A simple
      // demonstration appears also in "kdu_buffered_compress.cpp".
      //    A common question is how you can get the compressed data written
      // out to a memory buffer, instead of a file.  To do this, simply
      // derive a memory buffering object from the abstract base class,
      // `kdu_compressed_target' and pass this into `kdu_codestream::create'.
      // If you want a memory resident JP2/JPH file, pass your memory-buffered
      // `kdu_compressed_target' object to the second form of the overloaded
      // `jp2_family_tgt::open' function and proceed as before.
    kdu_core::kdu_codestream codestream; codestream.create(&siz,&output);

    // Set up any specific coding parameters and finalize them.
    //codestream.access_siz()->parse_string("Clayers=12");
    //codestream.access_siz()->parse_string("Creversible=yes -jp2_space sLUM Clayers=16 Cycc=no");
    //codestream.access_siz()->parse_string("Creversible=yes -jp2_space sLUM Cycc=no");
    codestream.access_siz()->parse_string("Creversible=yes");
    //codestream.access_siz()->parse_string("Cycc=no");
    //codestream.access_siz()->parse_string("-grey_weights");
    //codestream.access_siz()->parse_string("Cmodes=HT");

    //codestream.access_siz()->parse_string("Cmodes=HT");
    //codestream.access_siz()->parse_string("-grey_weights");
      // Other suggestions:
      // 1) Instead of "Creversible=yes", you could do lossy compression with a
      //    quality factor (similar to JPEG), using for example,
      //      codestream.access_siz()->parse_string("Qfactor=85");
      //      codestream.access_siz()->parse_string("Ctype=Y,Cb,Cr,N");
      //    for more details on how to set `Ctype' (component-type), see the
      //    `check_and_set_default_component_types' in "kdu_buffered_compress".
      // 2) Both with "Creversible=yes" or without it, and both with `Qfactor'
      //    or without it, you can specify the maximum size of the compressed
      //    result (or indeed of any or all quality layers) by passing this
      //    information to `compressor.start' below, via its optional arguments.
      //    If you do this, it is always a good idea to set `Ctype', since this
      //    allows Kakadu to perform visual optimization based on the component
      //    type, avoiding visual weighting of components whose type is "N"
      //    (non-visual).
      // 3) While `kdu_params::parse_string' is a simple and convenient function
      //    for accessing Kakadu's extensive parameter sub-system, you can
      //    directly set parameter values instead, using the
      //    `kdu_params::access_cluster', `kdu_params::access_relation' and
      //    `kdu_params::set' functions -- this relies upon knowing the cluster
      //    to which each parameter attribute belongs, as documented in
      //    "kdu_params.h" or by following the classes derived from `kdu_params'
      //    in Kakadu's auto-generated documentation system.

    codestream.access_siz()->finalize_all(); // Set up coding defaults

    // Now compress the image in one hit, using `kdu_stripe_compressor'
    kdu_supp::kdu_stripe_compressor compressor;
    compressor.start(codestream);
    int stripe_heights[3]={frameInfo_.height,frameInfo_.height,frameInfo_.height};
    compressor.push_stripe(decoded_.data(),stripe_heights);
    compressor.finish();
      // As an alternative to the above, you can read the image samples in
      // smaller stripes, calling `compressor.push_stripe' after each stripe.
      // Note that the `kdu_stripe_compressor' object can be used to realize
      // a wide range of Kakadu compression features, while supporting a wide
      // range of different application-selected stripe buffer configurations
      // precisions and bit-depths.  When developing your Kakadu-based
      // compression application, be sure to read through the extensive
      // documentation provided for this object.  For a much richer demonstration
      // of the use of `kdu_stripe_compressor', take a look at the
      // "kdu_buffered_compress" application.

    // Finally, cleanup
    codestream.destroy(); // All done: simple as that.
    tgt.close();
    output.close(); // Not really necessary here.
  }

private:
  std::vector<uint8_t> decoded_;
  std::vector<uint8_t> encoded_;
  FrameInfo frameInfo_;
  size_t decompositions_;
  bool lossless_;
  float quantizationStep_;
  size_t progressionOrder_;

  std::vector<Point> downSamples_;
  Point imageOffset_;
  Size tileSize_;
  Point tileOffset_;
  Size blockDimensions_;
  std::vector<Size> precincts_;
  bool isUsingColorTransform_;
};
