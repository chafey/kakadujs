# kakadujs
WASM Build of Kakadu

## Status

WIP - don't use yet

## Building

This code has been tested against v8_2_1 of Kakadu
This code requires that you have a license copy of Kakadu, you must place the
Kakadu source in the extern folder (e.g. extern/v_8_2_1-02044N)

To build native C/C++ version (Mac OS X x86 only - will not work for other platforms yet)
```
> ./build-native.sh
```

To build EMSCRIPTEN verision:

Launch docker container using Visual Studio Remote Containers and then:

```
> ./build.sh
```
