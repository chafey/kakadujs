#!/bin/sh

RED='\033[0;31m'
NC='\033[0m' # No Color

#rm -rf build-wasm
mkdir -p build-wasm
#(cd build-wasm && emcmake cmake -DCMAKE_BUILD_TYPE=Debug ..)
(cd build-wasm && emcmake cmake ..)
if [ $retVal -ne 0 ]; then
    echo "${RED}CMAKE FAILED${NC}"
    exit 1
fi

(cd build-wasm && emmake make VERBOSE=1 -j ${nprocs})
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "${RED}MAKE FAILED${NC}"
    exit 1
fi
mkdir -p ./dist
cp ./build-wasm/src/kakadujs.js ./dist
cp ./build-wasm/src/kakadujs.wasm ./dist
#(cd test/node; npm run test)
