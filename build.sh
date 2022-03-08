#!/bin/sh
rm -rf build
mkdir -p build
#(cd build && emcmake cmake -DCMAKE_BUILD_TYPE=Debug ..)
#(cd build && CXXFLAGS=-msimd128 emcmake cmake ..)
(cd build && emcmake cmake ..)
(cd build && emmake make VERBOSE=1 -j ${nprocs})
cp ./build/src/kakadujs.js ./dist
cp ./build/src/kakadujs.wasm ./dist
#(cd test/node; npm run test)
