#!/bin/bash

# this script does a clean build and is useful for development when modifying cmakefiles

rm -rf build-emscripten
emcmake cmake -S . -B build-emscripten -DCMAKE_BUILD_TYPE=Release  --preset=emscripten -DCMAKE_FIND_ROOT_PATH=/
(cd build-emscripten; emmake make -j)
cp ./build-emscripten/src/kakadujs.js ./dist
cp ./build-emscripten/src/kakadujs.wasm ./dist
build-emscripten/test/cpp/cpptest
