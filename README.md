# kakadujs
Kakadu wrapper for medical imaging use

Includes builds for WASM and native C/C++ on Mac/Linux x86/ARM64

## Status

WIP - don't use yet

## Building

This code has been developed/tested against v8_3 of Kakadu for Mac and Linux (x86 and arm).  Windows/visual studio
is possible but will require updates to the CMakeLists.txt file.  You must place a licensed version of the Kakadu source in the extern folder (e.g. extern/v_8_3-02044N) and update the variable KAKADU_ROOT in CMakeLists.txt accordingly.  To maximize performance, make sure you replace srclib_ht with altlib_ht_opt in the Kakadu directory (see
Enabling_HT.txt in the Kakadu library for more information)

### Prerequisites

* CMake
* C++ Compiler Toolchain (e.g. Ubuntu build-essentials, XCode command line tools)

### Building the native C++ version

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

## Building WASM version

Launch docker container using Visual Studio Remote Containers and then:

```
> ./build-wasm.sh
```
