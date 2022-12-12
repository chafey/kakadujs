# kakadujs
Kakadu wrapper for medical imaging use

Includes builds for WASM and native C/C++ on Mac/Linux/Windows

## Status

Experimental - use at your own risk

## Building

This code has been developed/tested against v8_3 of Kakadu for Mac (x64/ARM), Linux (x64/ARM) and Windows (x64).  
You must place a licensed version of the Kakadu source in the extern folder (e.g. extern/v_8_3-02044N) and update the
variable KAKADU_ROOT in CMakeLists.txt accordingly.  

NOTE - The CMake files are setup to automatically build with the processor SIMD optimizations without making any changes
to the default Kakadu source distribution.  You do not need to replace srclib_ht with altlib_ht_opt or set 
FC_BLOCK_ENABLED as described in the Kakadu/Enabling_HT.txt file - this is done automatically for you.  

### Prerequisites

#### Linux/Mac OS X

* CMake
* C++ Compiler Toolchain (e.g. Ubuntu build-essentials, XCode command line tools)

#### Windows

* Visual Studio 2022

### Building the native C++ version with Linux/Mac OS X

The test app in test/cpp/main.cpp will generate benchmarks for decoding and encoding.  TPF = Time Per Frame/Image.
Numbers from an Apple M1 MacBook Pro


```
> ./build-native.sh

NATIVE decode test/fixtures/j2c/CT1.j2c TotalTime: 0.014 s for 20 iterations; TPF=0.692 ms (361.12 MP/s, 1444.46 FPS)
NATIVE decode test/fixtures/j2c/MG1.j2c TotalTime: 0.744 s for 20 iterations; TPF=37.206 ms (374.94 MP/s, 26.88 FPS)
NATIVE encode test/fixtures/raw/CT1.RAW TotalTime: 0.037 s for 20 iterations; TPF=1.846 ms (135.44 MP/s, 541.74 FPS)
```

### Building the native C++ version with Windows/Visual Studio 2022

Build the x64-release version.  Run cpp test from the project root directory.  Numbers from an Intel 13900K CPU

```
C:\Users\chafe\source\repos\kakadujs>out\build\x64-Release\test\cpp\cpptest.exe
NATIVE decode test/fixtures/j2c/CT1.j2c TotalTime: 0.017 s for 20 iterations; TPF=0.850 ms (294.11 MP/s, 1176.43 FPS)
NATIVE decode test/fixtures/j2c/MG1.j2c TotalTime: 0.568 s for 20 iterations; TPF=28.400 ms (491.19 MP/s, 35.21 FPS)
NATIVE encode test/fixtures/raw/CT1.RAW TotalTime: 0.014 s for 20 iterations; TPF=0.700 ms (357.15 MP/s, 1428.61 FPS)
```

## Building WASM version

Launch docker container using Visual Studio Remote Containers and then:

```
> ./build-wasm.sh
WASM decode ../fixtures/j2c/CT1.j2c TotalTime: 0.100 s for 20 iterations; TPF=5.001 ms (49.99 MP/s, 199.95 FPS)
WASM decode ../fixtures/j2c/MG1.j2c TotalTime: 4.090 s for 20 iterations; TPF=204.477 ms (68.22 MP/s, 4.89 FPS)
WASM encode ../fixtures/raw/CT1.RAW TotalTime: 0.074 s for 20 iterations; TPF=3.710 ms (67.38 MP/s, 269.52 FPS)
```
