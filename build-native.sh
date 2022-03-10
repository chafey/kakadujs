#!/bin/sh

RED='\033[0;31m'
NC='\033[0m' # No Color

#rm -rf build-native
mkdir -p build-native
#(cd build-native && CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ..)
(cd build-native && CC=clang CXX=clang++ cmake ..)
if [ $retVal -ne 0 ]; then
    echo "${RED}CMAKE FAILED${NC}"
    exit 1
fi

(cd build-native && make VERBOSE=1 -j 8)
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "${RED}MAKE FAILED${NC}"
    exit 1
fi
(build-native/test/cpp/cpptest 20)  
