
if(EMSCRIPTEN)
  add_executable(kakadujs ${SOURCES} jslib.cpp)
  target_link_libraries(kakadujs PRIVATE libkdu)
  target_compile_features(kakadujs PUBLIC cxx_std_17)

  set_target_properties(
      kakadujs 
      PROPERTIES 
      LINK_FLAGS "\
        -O3 \
        -lembind \
        -s DISABLE_EXCEPTION_CATCHING=1 \
        -s ASSERTIONS=0 \
        -s NO_EXIT_RUNTIME=1 \
        -s MALLOC=emmalloc \
        -s ALLOW_MEMORY_GROWTH=1 \
        -s INITIAL_MEMORY=50MB \
        -s FILESYSTEM=0 \
        -s EXPORTED_FUNCTIONS=[] \
        -s EXPORTED_RUNTIME_METHODS=[ccall] \
    ")

  else()
    # TODO - turn this into a C++ library so it can be used in c++ applications.
    # Currently this is a header only lib so its not super useful but we could add
    # some non header code (.cpp) some day where this would be helpful
    #add_library(kakadujs ${SOURCES})
  endif()