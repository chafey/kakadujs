cmake_minimum_required(VERSION 3.20)

project (kakadujs LANGUAGES CXX)

# TODO: Update to the location of your Kakadu source code
set(KAKADU_ROOT "${CMAKE_SOURCE_DIR}/extern/v8_3-02120E")

# Enable SIMD by default
OPTION(KAKADU_SIMD_ACCELERATION "Enable Kakadu's heavily optimized implementation of HTJ2K" ON)

# Disable threading by default
OPTION(KAKADU_THREADING "Enable Kakadu multi-threaded decoding" OFF)

###########################

# set the build type if not specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}")
endif()

# do platform specific stuff
if(EMSCRIPTEN)
  SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Shared libraries forced off for EMSCRIPTEN" FORCE)  # EMSCRIPTEN does not support shared libraries
  SET(KAKADU_THREADING OFF CACHE BOOL "Kakadu threading forced off for EMSCRIPTEN" FORCE)  # EMSCRIPTEN threading has not been tested with kakadu yet
  SET(KAKADU_SIMD_ACCELERATION OFF CACHE BOOL "Kakadu SIMD acceleration forced off for EMSCRIPTEN" FORCE) # Kakadu does not support WASM-SIMD yet

  SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG") # O3 is the highest optimization level for EMSCRIPTEN
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msimd128") # enabled LLVM autovectoring for WASM SIMD
else()
  if(UNIX)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto") # add link time optimizations to release build

    # Note - shared libraries seems to slow down kakadu on Mac OS X at least...
    if(BUILD_SHARED_LIBS)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") # enable position independent code for shared libraries
    endif()

    EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
    message(STATUS "ARCHITECURE=${ARCHITECTURE}")
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin") # Mac
      add_compile_definitions(KDU_MAC_SPEEDUPS)  # enables cpu specific optimizations for mac (ARM | x86)
      if(ARCHITECTURE STREQUAL "arm64")
        set(KAKADU_PLATFORM "Mac-arm-64-gcc")
      else()
        set(KAKADU_PLATFORM "Mac-x86-64-gcc")
      endif()
    else() # Non mac unix (linux)
      if(ARCHITECTURE STREQUAL "aarch64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8.1-a") # TODO - try removing this and see if it builds (this was taken from kakadu makefile)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flax-vector-conversions") # TODO - try compiling without this (this was taken from kakadu makefile)
        set(KAKADU_PLATFORM "Linux-arm-64-gcc")
      else()
        set(KAKADU_PLATFORM "Linux-x86-64-gcc")
        add_compile_definitions(KDU_X86_INTRINSICS)  # enables x86 specific optimizations
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

# configure for compiler options and link directories based on SIMD Acceleration enabled or not
if(KAKADU_SIMD_ACCELERATION) 

  set(KAKDU_HT_LIB "altlib_ht_opt") # this has the CPU optimized libraryes (see Enabling_HT.txt)

else()
  add_compile_definitions(FBC_NO_ACCELERATION)# disable kakadu HT acceleration (see Enabling_HT.txt)

  set(KAKDU_HT_LIB "srclib_ht")  # this has the CPU optimized libraryes (see Enabling_HT.txt)
endif()

# enable HT mode in kakadu
add_compile_definitions(FBC_ENABLED)

# disable threads if not enabled
if(NOT KAKADU_THREADING)
  add_compile_definitions(KDU_NO_THREADS)
endif()

########################

# add the kakadu library from extern
add_subdirectory(extern/kakadu EXCLUDE_FROM_ALL)

# add the library code
add_subdirectory(src)

# c++ native test case
if(NOT EMSCRIPTEN)
  add_subdirectory(test/cpp)
endif()