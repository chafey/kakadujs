
set(PUBLIC_HEADERS
    "${KAKADU_ROOT}/coresys/common"
    "${KAKADU_ROOT}/coresys/shared"
    "${KAKADU_ROOT}/apps/support"
    "${KAKADU_ROOT}/apps/compressed_io"
)

include_directories(${PUBLIC_HEADERS})

set(CODING_SOURCES
    "${KAKADU_ROOT}/coresys/coding/block_coding_common.cpp"
    "${KAKADU_ROOT}/coresys/coding/block_decoder.cpp"
    "${KAKADU_ROOT}/coresys/coding/block_encoder.cpp"
    "${KAKADU_ROOT}/coresys/coding/decoder.cpp"
    "${KAKADU_ROOT}/coresys/coding/encoder.cpp"
    "${KAKADU_ROOT}/coresys/coding/cplex_analysis.cpp"
    "${KAKADU_ROOT}/coresys/coding/mq_decoder.cpp"
    "${KAKADU_ROOT}/coresys/coding/mq_encoder.cpp"
)

set(COMPRESSED_SOURCES
    "${KAKADU_ROOT}/coresys/compressed/blocks.cpp"
    "${KAKADU_ROOT}/coresys/compressed/codestream.cpp"
    "${KAKADU_ROOT}/coresys/compressed/compressed.cpp"
)
# v8_3 and later, remove for v8_2
set(SHARED_SOURCES
    "${KAKADU_ROOT}/coresys/shared/core_local.cpp"
)


set(KERNEL_SOURCES
    "${KAKADU_ROOT}/coresys/kernels/kernels.cpp"
)

set(MESSAGING_SOURCES
    "${KAKADU_ROOT}/coresys/messaging/messaging.cpp"
)

set(PARAMETERS_SOURCES
    "${KAKADU_ROOT}/coresys/parameters/params.cpp"
)

set(TRANSFORM_SOURCES
    "${KAKADU_ROOT}/coresys/transform/colour.cpp"
    "${KAKADU_ROOT}/coresys/transform/transform.cpp"
    "${KAKADU_ROOT}/coresys/transform/analysis.cpp"
    "${KAKADU_ROOT}/coresys/transform/synthesis.cpp"
    "${KAKADU_ROOT}/coresys/transform/fusion.cpp"
    "${KAKADU_ROOT}/coresys/transform/multi_transform.cpp"
    "${KAKADU_ROOT}/coresys/transform/analysis2.cpp"
    "${KAKADU_ROOT}/coresys/transform/synthesis2.cpp"
    "${KAKADU_ROOT}/coresys/transform/transform2.cpp"
)

set(ROI_SOURCES
    "${KAKADU_ROOT}/coresys/roi/roi.cpp"
)

set(COMMON_SOURCES
    "${KAKADU_ROOT}/coresys/common/kdu_arch.cpp"
    "${KAKADU_ROOT}/coresys/threads/kdu_threads.cpp"
)

set(FASTCODING_SOURCES
    "${KAKADU_ROOT}/coresys/fast_coding/fbc_common.cpp"
    "${KAKADU_ROOT}/coresys/fast_coding/fbc_encoder.cpp"
    "${KAKADU_ROOT}/coresys/fast_coding/fbc_decoder.cpp"
    "${KAKADU_ROOT}/coresys/fast_coding/fbc_encoder_tools.cpp"
    "${KAKADU_ROOT}/coresys/fast_coding/fbc_decoder_tools.cpp"
)

set(APP_SUPPORT_SOURCES
    "${KAKADU_ROOT}/apps/support/kdu_stripe_decompressor.cpp"
    "${KAKADU_ROOT}/apps/support/kdu_stripe_compressor.cpp"
    "${KAKADU_ROOT}/apps/support/supp_local.cpp"
    "${KAKADU_ROOT}/apps/jp2/jp2.cpp"
    "${KAKADU_ROOT}/apps/jp2/jpx.cpp"
    "${KAKADU_ROOT}/apps/client_server/kdu_client_window.cpp"
)

if(KAKADU_SIMD_ACCELERATION) 
    if(ARCHITECTURE STREQUAL "x86_64")

        set(SSSE3_SOURCES
            "${KAKADU_ROOT}/coresys/coding/ssse3_coder_local.cpp" 
            "${KAKADU_ROOT}/coresys/transform/ssse3_colour_local.cpp" 
            "${KAKADU_ROOT}/coresys/transform//ssse3_dwt_local.cpp"
        )

        set(SSE4_SOURCES
            "${KAKADU_ROOT}/coresys/transform/sse4_multi_transform_local.cpp" 
        )

        set(AVX_SOURCES
            "${KAKADU_ROOT}/coresys/coding/avx_coder_local.cpp"
            "${KAKADU_ROOT}/coresys/transform/avx_colour_local.cpp"
        )

        set(AVX2_SOURCES
            "${KAKADU_ROOT}/coresys/coding/avx2_coder_local.cpp"
            "${KAKADU_ROOT}/coresys/transform/avx2_colour_local.cpp"
            "${KAKADU_ROOT}/coresys/transform/avx2_dwt_local.cpp"
        )

        set(AVX2_X64_SOURCES
            "${KAKADU_ROOT}/coresys/transform/avx2_analysis2.cpp"
            "${KAKADU_ROOT}/coresys/transform/avx2_synthesis2.cpp"
        )

        set(APP_SUPPORT_SOURCES_INTEL_SIMD
            "${KAKADU_ROOT}/apps/support/avx2_stripe_transfer.cpp"
            "${KAKADU_ROOT}/apps/support/ssse3_stripe_transfer.cpp"
        )

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DKDU_NO_THREADS")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -DKDU_X86_INTRINSIC -m64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma -mbmi -mbmi2 -mlzcnt")
    elseif(ARCHITECTURE STREQUAL "arm64")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DKDU_NEON_INTRINSICS -DKDU_NO_THREADS")

        set(NEON_SOURCES
            "${KAKADU_ROOT}/apps/support/neon_stripe_transfer.cpp"
            "${KAKADU_ROOT}/apps/support/neon_region_decompressor.cpp"
            "${KAKADU_ROOT}/apps/support/neon_region_compositor.cpp"
            "${KAKADU_ROOT}/apps/image/neon_sample_reorg.cpp"
            "${KAKADU_ROOT}/coresys/coding/neon_coder_local.cpp"
            "${KAKADU_ROOT}/coresys/transform/neon_colour_local.cpp"
            "${KAKADU_ROOT}/coresys/transform/neon_dwt_local.cpp"
            "${KAKADU_ROOT}/coresys/transform/neon_multi_transform_local.cpp"
            "${KAKADU_ROOT}/coresys/transform/neon_synthesis2.cpp"
            "${KAKADU_ROOT}/coresys/transform/neon_analysis2.cpp"
        )
    endif()
 endif()

add_library (libkdu 
    ${SHARED_SOURCES}
    ${CODING_SOURCES} 
    ${COMPRESSED_SOURCES} 
    ${KERNEL_SOURCES} 
    ${MESSAGING_SOURCES}
    ${PARAMETERS_SOURCES}
    ${TRANSFORM_SOURCES}
    ${ROI_SOURCES}
    ${COMMON_SOURCES}
    ${FASTCODING_SOURCES}
    ${SSSE3_SOURCES} 
    ${SSE4_SOURCES} 
    ${AVX_SOURCES} 
    ${AVX2_SOURCES} 
    ${AVX2_X64_SOURCES}
    ${HT_FASTCODING_SOURCES}
    ${APP_SUPPORT_SOURCES}
    ${APP_SUPPORT_SOURCES_INTEL_SIMD}
    ${NEON_SOURCES}
    )


target_include_directories(libkdu PUBLIC ${PUBLIC_HEADERS})


target_compile_features(libkdu PUBLIC cxx_std_17)
