
if(EMSCRIPTEN)
  add_executable(kakadujs ${SOURCES} jslib.cpp)
  target_link_libraries(kakadujs PRIVATE libkdu)
  target_compile_features(kakadujs PUBLIC cxx_std_17)
else()
  #add_library(kakadujs ${SOURCES})
endif()

if(EMSCRIPTEN)

  if (CMAKE_BUILD_TYPE STREQUAL Debug)
    SET(linkFlags "-g4")
  else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
    SET(linkFlags "-O3")
  endif()

  set_target_properties(
      kakadujs 
      PROPERTIES 
      LINK_FLAGS "\
        --bind \
        -s DISABLE_EXCEPTION_CATCHING=1 \
        -s ASSERTIONS=0 \
        -s NO_EXIT_RUNTIME=1 \
        -s MALLOC=emmalloc \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s TOTAL_MEMORY=536870912 \
        -s FILESYSTEM=0 \
        -s EXPORTED_FUNCTIONS=[] \
        -s EXPORTED_RUNTIME_METHODS=[ccall] \
    ")
    #        -s WASM_BIGINT \

endif()
