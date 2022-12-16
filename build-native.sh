#!/bin/bash

rm -rf build-native
mkdir -p build-native
#(cd build-native && CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ..)
(cd build-native && cmake .. --preset=clang)
#(cd build-native && cmake ..)
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "CMAKE FAILED"
    exit 1
fi

#(cd build-native && make -j $NPROC)
(cd build-native && make VERBOSE=1 -j)
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "MAKE FAILED"
    exit 1
fi
(build-native/test/cpp/cpptest)  
