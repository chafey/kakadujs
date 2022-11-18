# kakadujs
Kakadu wrapper for medical imaging use

Includes builds for WASM and native C/C++ on Mac/Linux/Windows

## Status

Experimental - use at your own risk

## Building

This code has been developed/tested against v8_3 of Kakadu for Mac (x64/ARM), Linux (x64/ARM) and Windows (x64).  
You must place a licensed version of the Kakadu source in the extern folder (e.g. extern/v_8_3-02044N) and update the
variable KAKADU_ROOT in CMakeLists.txt accordingly.  To maximize performance, make sure you replace srclib_ht with 
altlib_ht_opt in the Kakadu directory (see Enabling_HT.txt in the Kakadu library for more information)

### Prerequisites

### Linux/Mac OS X

* CMake
* C++ Compiler Toolchain (e.g. Ubuntu build-essentials, XCode command line tools)

### Windows

* Visual Studio 2022

### Building the native C++ version with Linux/Mac OS X

The test app in test/cpp/main.cpp will generate benchmarks for decoding and encoding.  TPF = Time Per Frame/Image.
Ignore the first two lines as they are just warming up the decoder/encoder for more accurate numbers below.

```
> ./build-native.sh

Native-decode test/fixtures/j2c/CT1.j2c TotalTime= 1.58 ms TPF=1.58 ms (158.43 MP/s, 633.71 FPS)
FrameInfo 512x512x1 16 bpp
Native-encode test/fixtures/raw/CT1.RAW Size=185400 TotalTime= 2.83 ms TPF=2.83 ms (88.21 MP/s, 352.86 FPS)
Native-decode test/fixtures/j2c/CT1.j2c TotalTime= 25.68 ms TPF=0.86 ms (292.09 MP/s, 1168.36 FPS)
Native-decode test/fixtures/j2c/MG1.j2c TotalTime= 1094.22 ms TPF=36.47 ms (382.46 MP/s, 27.42 FPS)
FrameInfo 512x512x1 16 bpp
Native-encode test/fixtures/raw/CT1.RAW Size=185400 TotalTime= 55.38 ms TPF=1.85 ms (135.42 MP/s, 541.68 FPS)
Native-decode test/fixtures/j2k/US1.j2k TotalTime= 21.70 ms TPF=21.70 ms (13.50 MP/s, 46.08 FPS)
FrameInfo 512x512x1 16 bpp
Native-encode test/fixtures/raw/CT1.RAW Size=185400 TotalTime= 54.42 ms TPF=1.81 ms (137.81 MP/s, 551.23 FPS)
decoding test/fixtures/raw/CT1.RAW
```

### Building the native C++ version with Windows/Visual Studio 2022

Build the x64-release version.  Run cpp test from the project root directory

```
C:\Users\chafe\source\repos\kakadujs>out\build\x64-Release\test\cpp\cpptest
Native-decode test/fixtures/j2c/CT1.j2c TotalTime= 3.00 ms TPF=3.00 ms (83.32 MP/s, 333.27 FPS)
FrameInfo 512x512x1 16 bpp
Native-encode test/fixtures/raw/CT1.RAW Size=185400 TotalTime= 2.00 ms TPF=2.00 ms (125.01 MP/s, 500.03 FPS)
Native-decode test/fixtures/j2c/CT1.j2c TotalTime= 2.00 ms TPF=2.00 ms (124.95 MP/s, 499.80 FPS)
Native-decode test/fixtures/j2c/MG1.j2c TotalTime= 78.02 ms TPF=78.02 ms (178.81 MP/s, 12.82 FPS)
FrameInfo 512x512x1 16 bpp
Native-encode test/fixtures/raw/CT1.RAW Size=185400 TotalTime= 2.00 ms TPF=2.00 ms (124.98 MP/s, 499.93 FPS)
Native-decode test/fixtures/j2k/US1.j2k TotalTime= 33.01 ms TPF=33.01 ms (8.88 MP/s, 30.30 FPS)
FrameInfo 512x512x1 16 bpp
Native-encode test/fixtures/raw/CT1.RAW Size=185400 TotalTime= 2.00 ms TPF=2.00 ms (124.97 MP/s, 499.88 FPS)
decoding test/fixtures/raw/CT1.RAW
```

## Building WASM version

Launch docker container using Visual Studio Remote Containers and then:

```
> ./build-wasm.sh
```
