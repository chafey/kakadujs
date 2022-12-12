cmake_minimum_required(VERSION 3.16)

project (kakadujs LANGUAGES CXX)

# TODO: Update to the location of your Kakadu source code
set(KAKADU_ROOT "${CMAKE_SOURCE_DIR}/extern/v8_3-02120E")

# set the build type if not specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}")
endif()


# use kakadu fast block coder (HTJ2K)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFBC_ENABLED")
set(KAKDU_HT_LIB "altlib_ht_opt")
set(KAKADU_SIMD_ACCELERATION ON)

# do platform specific stuff
if(EMSCRIPTEN)
  SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msimd128")
else()
  if(UNIX)
    # warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -Wno-overloaded-virtual")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-return-type")

    # Use -Ofast to maximize speed
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Ofast -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
    message(STATUS "ARCHITECURE=${ARCHITECTURE}")
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKDU_MAC_SPEEDUPS")
      if(ARCHITECTURE STREQUAL "arm64")
        set(KAKADU_PLATFORM "Mac-arm-64-gcc")
      else()
        set(KAKADU_PLATFORM "Mac-x86-64-gcc")
      endif()
    else()
      if(ARCHITECTURE STREQUAL "aarch64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8.1-a -flax-vector-conversions")
        set(KAKADU_PLATFORM "Linux-arm-64-gcc")
      else()
        set(KAKADU_PLATFORM "Linux-x86-64-gcc")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKDU_X86_INTRINSICS")
      endif()
    endif()
  elseif(WIN32)
    set(ARCHITECTURE "x86_64")
    add_compile_definitions(KDU_X86_INTRINSICS)
    set(KAKADU_PLATFORM "Win-x86-64")
  else()
    message(FATAL_ERROR "Platform not supported yet" )
  endif()
endif()


# turn off threading
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKDU_NO_THREADS")

# BUILD_SHARED_LIBS must be OFF for EMSCRIPTEN builds
option(BUILD_SHARED_LIBS "" OFF)

# add the external library
add_subdirectory(extern/kakadu EXCLUDE_FROM_ALL)

# add the js wrapper
add_subdirectory(src)

# c++ native test case
if(NOT EMSCRIPTEN)
  add_subdirectory(test/cpp)
endif()