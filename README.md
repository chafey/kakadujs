# kakadujs
WASM Build of Kakadu

## Status

WIP - don't use yet

## Building

This code has been tested against v8_2_1 of Kakadu.  You must place a licensed version of the Kakadu source in the extern folder (e.g. extern/v_8_2_1-02044N).
To maximize performance, make sure you replace srclib_ht with altlib_ht_opt as per instructions in Enabling_HT.txt.

NOTE: This should only work on Mac OS X x86 since there are platform specific settings in the CMakefiles.  This will be fixed at some point

To build native C/C++ version
```
> ./build-native.sh
```

To build EMSCRIPTEN verision:

Launch docker container using Visual Studio Remote Containers and then:

```
> ./build.sh
```
