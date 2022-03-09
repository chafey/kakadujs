cmake_minimum_required(VERSION 3.16)

project (kakadujs
  LANGUAGES CXX)

# set the build type if not specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}")
endif()

# use kakadu fast block decoding
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFBC_ENABLED")

# turn off threading
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKDU_NO_THREADS")

# warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -Wno-return-type  -Wno-misleading-indentation")

if(EMSCRIPTEN)
  SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msimd128")
  option(KAKADU_INTEL_SIMD "" OFF)
else()
  set(CMAKE_CXX_FLAGS "-DNDEBUG -Ofast -fPIC -arch x86_64")
option(KAKADU_INTEL_SIMD "" ON)
endif()

set(KAKADU_ROOT "${CMAKE_SOURCE_DIR}/extern/v8_2_1-02044N")

option(BUILD_SHARED_LIBS "" OFF)

# add the external library
add_subdirectory(extern/kakadu EXCLUDE_FROM_ALL)

# add the js wrapper
add_subdirectory(src)

# c++ native test case
if(NOT EMSCRIPTEN)
  add_subdirectory(test/cpp)
endif()