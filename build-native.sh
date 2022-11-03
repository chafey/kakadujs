#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No Color

rm -rf build-native
mkdir -p build-native
#(cd build-native && CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ..)
(cd build-native && cmake ..)
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "${RED}CMAKE FAILED${NC}"
    exit 1
fi
NPROC=$(sysctl -n hw.ncpu)
if [ -z $NPROC ]; then
    NPROC=$(nproc)
fi

#(cd build-native && make -j $NPROC)
(cd build-native && make VERBOSE=1 -j $NPROC)
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "${RED}MAKE FAILED${NC}"
    exit 1
fi
(build-native/test/cpp/cpptest 30)  
