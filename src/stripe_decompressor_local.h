/*****************************************************************************/
// File: stripe_decompressor_local.h [scope = APPS/SUPPORT]
// Version: Kakadu, V8.1
// Author: David Taubman
// Last Revised: 19 February, 2021
/*****************************************************************************/
// Copyright 2001, David Taubman.  The copyright to this file is owned by
// Kakadu R&D Pty Ltd and is licensed through Kakadu Software Pty Ltd.
// Neither this copyright statement, nor the licensing details below
// may be removed from this file or dissociated from its contents.
/*****************************************************************************/
// Licensee: HTJ2K
// License number: 02044
// The licensee has been granted a (non-HT) INDIVIDUAL NON-COMMERCIAL license
// to the contents of this source file.  A brief summary of this license appears
// below.  This summary is not to be relied upon in preference to the full
// text of the license agreement, accepted at purchase of the license.
// 1. The Licensee has the right to install and use the Kakadu software and
//    to develop Applications for the Licensee's own individual use.
// 2. The Licensee has the right to Deploy Applications built using the
//    Kakadu software to Third Parties, so long as such Deployment does not
//    result in any direct or indirect financial return to the Licensee or
//    any other Third Party, which further supplies or otherwise uses such
//    Applications, and provided Kakadu's HT block encoder/decoder
//    implementation remains disabled, unless explicit permission has been
//    granted to the Licensee to deploy Applications with HT enabled.
// 3. The Licensee has the right to distribute Reusable Code (including
//    source code and dynamically or statically linked libraries) to a Third
//    Party, provided the Third Party possesses a suitable license to use the
//    Kakadu software, and provided such distribution does not result in any
//    direct or indirect financial return to the Licensee.
// 4. The Licensee has the right to enable Kakadu's HT block encoder/decoder
//    implementation for evaluation and personal development purposes, but
//    not for deployed Applications.
/******************************************************************************
Description:
   Local definitions used in the implementation of the
`kdu_stripe_compressor' object.
******************************************************************************/

#ifndef STRIPE_DECOMPRESSOR_LOCAL_H
#define STRIPE_DECOMPRESSOR_LOCAL_H

#include "kdu_stripe_decompressor.h"
#include "supp_local.h"

// Objects declared here:
namespace kd_supp_local { 
  struct kdsd_component_state;
  struct kdsd_component;
  struct kdsd_tile;
  struct kdsd_queue;
}

#define KDSD_BUF8      0
#define KDSD_BUF16     1
#define KDSD_BUF32     2
#define KDSD_BUF_FLOAT 6

// Configure processor-specific compilation options
#if (defined KDU_PENTIUM_MSVC)
#  undef KDU_PENTIUM_MSVC
#  ifndef KDU_X86_INTRINSICS
#    define KDU_X86_INTRINSICS // Use portable intrinsics instead
#  endif
#endif // KDU_PENTIUM_MSVC

#if defined KDU_X86_INTRINSICS
#  include "x86_stripe_transfer_local.h"
#  define KDU_SIMD_OPTIMIZATIONS
#elif defined KDU_NEON_INTRINSICS
#  include "neon_stripe_transfer_local.h"
#  define KDU_SIMD_OPTIMIZATIONS
#endif

namespace kd_supp_local { 
  using namespace kdu_supp;

#ifdef KDU_SIMD_OPTIMIZATIONS
  using namespace kd_supp_simd;
#endif


/* ========================================================================= */
/*                      ACCELERATION FUNCTION POINTERS                       */
/* ========================================================================= */

typedef void
  (*kdsd_simd_transfer_func)(void *dst, void **src, int width,
                             int precision, int orig_precision,
                             bool is_absolute, bool dst_signed,
                             int store_preferences);
  /* Note: `store_preferences' passes flags that may (optionally) affect
     the way in which data is stored.  Currently, the only defined preference
     is `KDU_STRIPE_STORE_PREF_STREAMING', which encourages the vectorized
     data transfer functions to store their results in the `dst' buffer
     using aligned non-temporal vector stores -- the intention is to write
     around the cache, avoiding cache pollution, but potentially increasing
     the delay associated with subsequently reading the stored data back
     into another part of the application. */

/*****************************************************************************/
/*                            kdsd_component_state                           */
/*****************************************************************************/

struct kdsd_component_state { 
  private: // Prevent explicit new/delete calls.
#ifdef KDU_HAVE_C11
    kdsd_component_state() = default; // These are better; allows memset, etc.
    ~kdsd_component_state() = default;
#else // prior to C++11
    kdsd_component_state() {}
    ~kdsd_component_state() {}
#endif // prior to C++11
  public: // Array create/destroy funcs
    static kdsd_component_state *create_n(kd_suppmem *smem, size_t num_elts)
      { // No meaningful constructor, so we use calloc_struct to zero all mbrs
        return (kdsd_component_state *)
          smem->calloc_structs(sizeof(kdsd_component_state),num_elts);
      }
    void destroy_n(kd_suppmem *smem)
      { smem->free(this); }
  public: // Member functions
    void update(kdu_coords next_tile_idx, kdu_codestream codestream);
      /* Called immediately after processing stripe data for a row of tiles.
         Adjusts the values of `remaining_tile_height', `next_tile_height'
         and `stripe_height' accordingly, while also updating the `buf8',
         `buf16', `buf32' or `buf_float' pointers to address the start of
         the next row of tiles.
         If `remaining_tile_height' is reduced to 0, the function
         copies `next_tile_height' into `remaining_tile_height', decrements
         `remaining_tile_rows' and then uses the codestream interface together
         with `next_tile_idx' to determine a new value for
         `next_tile_height'. */
  public: // Data members that hold global information
    int comp_idx;
    int pos_x; // x-coord of left-most sample in the component
    int width; // Full width of the component
    int original_precision; // Precision recorded in SIZ marker segment
    kdu_coords sub_sampling; // Component sub-sampling factors from codestream
  public: // Data members that hold stripe-specific state information
    int row_gap, sample_gap, precision; // Values supplied by `pull_stripe'
    bool is_signed; // Value supplied by `pull_stripe'
    int buf_type; // One of `KDSD_BUFxxx'; note, 2 lsbs hold log2(bytes/sample)
    union { 
      kdu_byte *buf_ptr; // Emphasizes the presence of an anonymous union
      kdu_byte *buf8;   // if `buf_type'=`KDSD_BUF8'=0
      kdu_int16 *buf16; // if `buf_type'=`KDSD_BUF16'=1
      kdu_int32 *buf32; // if `buf_type'=`KDSD_BUF32'=2
      float *buf_float; // if `buf_type'=`KDSD_BUF_FLOAT'=6
    };
    int pad_flags; // See `pad_flags' argument to `pull_stripe'
    int stripe_height; // Remaining height in the current stripe
    int remaining_tile_height; // See below
    int next_tile_height; // See below
    int max_tile_height;
    int max_recommended_stripe_height;
    int remaining_tile_rows; // See below -- by "rows" we mean rows of tiles
    int y_tile_idx; // Absolute vertical index of the current row of tiles
  };
  /* Notes:
       `stripe_height' holds the total number of rows in the current stripe
     which have not yet been fully processed.  This value is updated at the
     end of each row of tiles, by subtracting the smaller of `stripe_height'
     and `remaining_tile_height'.
       `remaining_tile_height' holds the number of rows in the current
     row of tiles, which have not yet been fully processed.  This value
     is updated at the end of each row of tiles, by subtracting the smaller
     of `stripe_height' and `remaining_tile_height'.
       `remaining_tile_rows' holds the number of tile rows that have not
     yet been fully processed.  It is decremented each time
     `remaining_tile_height' goes to 0.  The `update' function uses this
     to figure out how to set `next_tile_height'.
       `next_tile_height' holds the value that will be moved into
     `remaining_tile_height' when we advance to the next row of tiles.  It
     reflects the total height of the tile that is immediately below the
     current one -- 0 if there is no such tile.
       `max_tile_height' is the maximum height of any tile in the image.
     In practice, this is the maximum of the heights of the first and
     second vertical tiles, if there are multiple tiles.
       `max_recommended_stripe_height' remembers the value returned by first
     call to `kdu_stripe_decompressor::get_recommended_stripe_heights'. */

/*****************************************************************************/
/*                             kdsd_component                                */
/*****************************************************************************/

struct kdsd_component { 
    // Manages processing of a single tile-component.
  private: // Prevent explicit new/delete calls.
#ifdef KDU_HAVE_C11
    kdsd_component() = default; // These are better; allows memset, etc.
    ~kdsd_component() = default;
#else // prior to C++11
    kdsd_component() {}
    ~kdsd_component() {}
#endif // prior to C++11
  public: // Array create/destroy funcs
    static kdsd_component *create_n(kd_suppmem *smem, size_t num_elts)
      { // No non-zeroing constructor; use calloc_struct to zero all mbrs
        return (kdsd_component *)
          smem->calloc_structs(sizeof(kdsd_component),num_elts);
      }
    void destroy_n(kd_suppmem *smem)
      { // Invoked on the first object in an array -- or just one object, or
        // even an empty array - does not read any members if length is 0.
#ifdef KDU_SIMD_OPTIMIZATIONS
        size_t num_elts = smem->get_num_elts(this,sizeof(*this));
        kdsd_component *comp = this;
        for (; num_elts != 0; num_elts--, comp++)
          if (comp->simd_pad_handle)
            { smem->free(comp->simd_pad_handle); comp->simd_pad_handle=NULL; }
#endif // KDU_SIMD_OPTIMIZATIONS
        smem->free(this); // No meaningful destructor action
      }
  public: // Data configured by `kdsd_tile::init' for a new tile
    kdu_coords size; // Tile width, by tile rows left in tile
    bool using_shorts; // If `kdu_line_buf's hold 16-bit samples
    bool is_absolute; // If `kdu_line_buf's hold absolute integers
    int horizontal_offset; // From left edge of image component
    int ratio_counter; // See below
  public: // Data configured by `kdsd_tile::init' for a new or existing tile
    int stripe_rows_left; // Counts down from `stripe_rows' during processing
    int sample_gap; // For current stripe being processed by `pull_stripe'
    int row_gap; // For current stripe being processed by `pull_stripe'
    int precision; // For current stripe being processed by `pull_stripe'
    bool is_signed; // For current stripe being processed by `pull_stripe'
    int buf_type; // One of `KDSD_BUFxxx'; note, 2 lsbs hold log2(bytes/sample)
    union { 
      kdu_byte *buf_ptr; // Emphasizes the presence of an anonymous union
      kdu_byte *buf8;   // if `buf_type'=`KDSD_BUF8'=0
      kdu_int16 *buf16; // if `buf_type'=`KDSD_BUF16'=1
      kdu_int32 *buf32; // if `buf_type'=`KDSD_BUF32'=2
      float *buf_float; // if `buf_type'=`KDSD_BUF_FLOAT'=6
    };
    int pad_flags; // For current stripe being processed by `pull_stripe'
  public: // Data configured by `kdu_stripe_decompressor::get_new_tile'
    int original_precision; // Original sample precision
    int vert_subsampling; // Vertical sub-sampling factor of this component
    int count_delta; // See below
  public: // Acceleration functions
#ifdef KDU_SIMD_OPTIMIZATIONS
    kdsd_simd_transfer_func  simd_transfer; // See below
    kdsd_component *simd_grp; // See below
    int simd_ilv; // See below
    int simd_padded_ilv; // See below
    kdu_int32 *simd_pad_handle; // Deallocation handle to padding source buffer
    kdu_int32 *simd_pad_buf; // Large enough to emulate `kdu_sample32'
    int simd_pad_buf_elts; // Facilitate reallocation if necessary
    void *simd_src[4]; // See below
    int simd_store_preferences; // Passed as last arg to `simd_transfer'
#endif
  };
  /* Notes:
        The `stripe_rows_left' member holds the number of rows in the current
     stripe, which are still to be processed by this tile-component.
        There are four buffer types that may be used, depending on the value
     of the `buf_type' member.  To facilitate generic manipulation of buffer
     pointers (without having to write code to cover each specific case),
     all buffer pointers are stored in an anonymous union and the two
     LSB's of `buf_type' are guranteed to hold log_2(bytes/sample).  Thus,
     the two LSB's of `buf_type' hold 0 for 8-bit sample buffers, 1 for
     16-bit sample buffers, and 2 for 32-bit integer and floating point
     sample buffers.
        The current buffer pointer (`buf8', `buf16', `buf32' or `buf_float'
     as appropriate), points to the start of the first unprocessed row of the
     current tile-component.
        The `remaining_buf_elts' member plays the same role as its namesake
     in `kdsd_component_state', with reference to the `buf8', `buf16',
     `buf32' or `buf_float' pointer that appears here -- this may have been
     offset.  The member is decremented by `row_gap' each time the relevant
     buffer pointer is incremented by `row_gap'.  In this way, the
     individual sample transfer procedures can determine whether or not
     a vector transfer might overwrite the buffer that was ultimately
     provided by the application.
        The `ratio_counter' is initialized to 0 at the start of each stripe
     and decremented by `count_delta' each time we consider processing this
     component within the tile.  Once the counter becomes negative, a new row
     from the stripe is processed and the ratio counter is subsequently
     incremented by `vert_subsampling'.  The value of the `count_delta'
     member is the minimum of the vertical sub-sampling factors for all
     components.  This policy ensures that we process the components in a
     proportional way.
        The `simd_transfer' function pointer, along with the other
     `simd_xxx' members, are configured by `kdsd_tile::init' based on
     functions that might be available to handle the particular conversion
     configuration for a tile-component (or collection of tile-components) in
     a vectorized manner.
        `simd_grp', `simd_ilv' and `simd_src' work together
     to support efficient transfer to interleaved component buffers, but
     the interpretation works also for non-interleaved buffers.  They
     have the following meanings:
     -- `simd_grp' points to the last component in an interleaved group; if
        NULL there is no SID implementation; the `simd_transfer' function
        is not called until we reach that component.
     -- `simd_ilv' holds the location that the current image component
        occupies within the `simd_grp' object's `simd_src' array -- the
        address of the current `kdu_line_buf' object's internal array should
        be written to that entry prior to any call to the `simd_transfer'
        function.
   */

/*****************************************************************************/
/*                               kdsd_tile                                   */
/*****************************************************************************/

struct kdsd_tile { 
  public: // Member functions
    static void *operator new(size_t nbytes, kd_suppmem *smem)
      { return smem->alloc(nbytes,KDU_ALIGNOF(kdsd_tile,8)); }
    static void operator delete(void *ptr, kd_suppmem *smem)
      { return smem->free(ptr); } // Called only if construct throws
    kdsd_tile(int allocator_frag_bits, kdu_membroker *membroker,
              kd_suppmem *smem)
      { 
        this->suppmem = smem;
        num_components=0; components=NULL; next=NULL; queue=NULL;
        sample_allocator.configure(membroker,allocator_frag_bits);
      }
  private: // Prevent expicit invocation of destructor
    ~kdsd_tile()
      { 
        if (components != NULL)
          { components->destroy_n(suppmem); components = NULL; }
        if (engine.exists()) engine.destroy();
      }
  public: // Back to public member functions
    void destroy()
      { /* MUST use this instead of `delete'!! */
        kd_suppmem *smem = suppmem;
        this->~kdsd_tile();
        smem->free(this);
      }
    void configure(int num_comps, const kdsd_component_state *comp_states);
      /* This function must be called after construction or whenever the
         number of components or their individual attributes (original
         precision or sub-sampling) may have changed.  In practice, the
         function is always invoked when a tile is first constructed or
         retrieved from the `kdu_stripe_decompressor::free_tiles' list. */
    void create(kdu_coords idx, kdu_codestream codestream,
                kdsd_component_state *comp_states, bool force_precise,
                bool want_fastest, kdu_thread_env *env,
                int env_dbuf_height, kdsd_queue *env_queue,
                const kdu_push_pull_params *pp_params, int tiles_wide);
      /* This function creates the tile processing engine, making it available
         to worker threads to start generating data.  However, the function
         does not associate the tile with stripe buffers.  That is done
         by the `init' function, which must be called right before any call
         to `process'.  When calling this function, it is expected that the
         internal `tile' interface is initially empty.
            In a multi-threaded setting (i.e., where `env' is non-NULL), you
         are required to supply a non-NULL `env_queue' argument, representing
         the queue to which the tile engine is supposed to belong.  If the
         function is creating the tile processing engine for the first time,
         it should find the `queue' member to be NULL, so it changes
         `queue' to point to `env_queue' and makes sure that the `env_queue'
         identifies us as one of its members (demarcated by the
         `kdsc_queue::first_tile' and `kdsc_queue::last_tile' members).
         Otherwise, the function only verifies that `queue' is identical to
         `env_queue'.
            If `env_queue' is non-NULL, the `KDU_MULTI_XFORM_DELAYED_START'
         flag is passed to `kdu_multi_synthesis::create', meaning that the
         `kdsd_queue::start' function needs to be called later to finish
         starting up all the tile processing engines that share the same
         queue.
            When `env' is non-NULL, it is also expected that the tile has
         previously been scheduled for opening in the background, so this
         function uses `kdu_codestream::access_tile' instead of
         `kdu_codestream::open_tile'.  This background scheduling of tile
         open operations is actually managed by the
         `kdsc_stripe_decompressor::augment_started_tiles' function.
            The `tiles_wide' argument is used by the internal algorithm to
         determine whether a negative `env_dbuf_height' argument should be
         passed straight through to `kdu_multi_synthesis::start' to choose
         its own default double buffering strategy or instead set to a
         value which is large enough to accommodate complete buffering of
         the entire tile's data.  The latter is appropriate if the
         codestream has multiple horizontally adjacent tiles and the
         current stripe being pulled from the `kdu_stripe_compressor::pull'
         function spans the entire tile.  In this special case we would like
         to make sure that the `pull' call does not get blocked waiting for
         a first tile to produce all of its data before we get to start the
         following tile -- we normally keep multiple concurrent tile processing
         engines active even if we do not keep enough tile engines to span an
         entire row of tiles. */
    void init(kdsd_component_state *comp_states, int store_prefs);
      /* Initializes an already created tile with a new set of stripe buffers
         and associated parameters.  The `tile' interface should not be empty,
         because `create' should have been called previously.  This allows you
         to create multiple tile processing engines ahead of time and then
         initialize and pull data from them one by one.
            The buffer pointer (all buffer types are aliased in an anonymous
         union) of each element in the `comp_states' array corresponds to the
         first sample in what remains of the current stripe, within each
         component.  This first sample is aligned at the left edge of the
         image.  The function determines the amount by which the buffer should
         be advanced to find the first sample in the current tile, by comparing
         the horizontal location of each tile-component, as returned by
         `kdu_tile_comp::get_dims', with the values in the `pos_x' members of
         the `kdsd_component_state' entries in the `comp_states' array.
            The `store_prefs' argument is the `vectorized_store_prefs' argument
         passed to the `pull_stripe' function. `*/
    bool process(kdu_thread_env *env);
      /* Processes all the stripe rows available to tile-components in the
         current tile, returning true if the tile is completed and false
         otherwise.  Always call this function right after `init'.
         When the function returns true, the tile has been fully processed,
         which means that it can be moved to the `inactive_tiles' list,
         except where it belongs to a thread queue (`queue' non-NULL), in
         which case this is done by `kdu_stripe_decompressor::release_queue'
         as soon as all tiles in the queue have been finished. */
    void close_tile_interface(kdu_thread_env *env)
      { // Can be done before `cleanup' if desired.
        assert(this->queue == NULL);
        tile.close(env,true); // Schedules for background closure; no hold-ups
        tile = kdu_tile(NULL); // Not really necessary; `close' also does this
      }
    void cleanup(kdu_thread_env *env)
      { 
        assert(queue == NULL);
        close_tile_interface(env);
        engine.destroy();
      }
  public: // Data
    kdu_tile tile;
    kdu_multi_synthesis engine;
    kdu_sample_allocator sample_allocator; // Used with `engine'
    kdsd_tile *next; // Next free tile, or next partially completed tile
    kdsd_queue *queue; // Non-NULL only for multi-threaded processing
    kd_suppmem *suppmem;
  public: // Data configured by `kdu_stripe_decompressor::get_new_tile'.
    int num_components;
    kdsd_component *components; // Array of tile-components
  };

/*****************************************************************************/
/*                                kdsd_queue                                 */
/*****************************************************************************/

struct kdsd_queue { 
  public: // Member functions
      static void *operator new(size_t nbytes, kd_suppmem *smem)
      { return smem->alloc(nbytes,KDU_ALIGNOF(kdsd_queue,8)); }
    static void operator delete(void *ptr, kd_suppmem *smem)
      { return smem->free(ptr); } // Called only if construct throws
    kdsd_queue()
      { first_tile = last_tile = NULL; next = NULL; num_tiles=0; }
  private: // Prevent explicit invocation of the destructor
    ~kdsd_queue()
      { assert(!thread_queue.is_attached()); }
  public: // Back to public member functions
    void destroy(kd_suppmem *smem)
      { /* MUST use this instead of `delete'!! */
        this->~kdsd_queue();
        smem->free(this);
      }
    void start(kdu_thread_env *env)
      { 
        bool fully_started = false;
        while (!fully_started)
          { 
            fully_started = true; // Until we know any better
            for (kdsd_tile *tp=first_tile; tp != NULL; tp=tp->next)
              { 
                if (!tp->engine.start(env))
                  fully_started = false; // Need to go around again
                if (tp == last_tile)
                  break;
              }
          }
      }
      /* Tiles that are created with a `kdsd_queue' object have their
         `kdu_multi_synthesis::create' function invoked with the
         `KDU_MULTI_XFORM_DELAYED_START' option, after which this function
         should be called to finish starting the multi-threaded processing
         jobs within each of the queue's tile processing engines.  This
         helps to cleanly interleave the jobs scheduled by each engine
         that lives within the same queue -- these all have the same priority
         as determined by the queue's sequence index. */
  public: // Data
    kdu_thread_queue thread_queue;
    kdsd_tile *first_tile;
    kdsd_tile *last_tile;
    int num_tiles; // Number of tiles associated with this queue
    kdsd_queue *next;
  };
  /* Notes:
       This object is required only for multi-threaded processing.  It
       serves to associate a collection of tile processing engines with a
       single queue that can be waited upon (joined).  This is particularly
       interesting for multi-tile images.
          If sample data is pulled from tiles sequentially (all of one tile,
       then all of the next, and so forth), each tile gets its own queue and
       we generally delay waiting upon the queue.
          If sample data is pulled from a whole row of tiles together, in
       interleaved fashion (one or more lines of one tile, then the next, etc.,
       then back to pull more lines from the first tile on the row, and so
       forth), the entire row of tile processing engines is assigned to a
       single queue.
  */

} // namespace kd_supp_local

#endif // STRIPE_DECOMPRESSOR_LOCAL_H
