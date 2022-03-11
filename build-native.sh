#!/bin/bash
rm -rf build-native
mkdir -p build-native
#(cd build-native && cmake -DCMAKE_BUILD_TYPE=Debug ..)
(cd build-native && cmake ..)
NPROC=$(sysctl -n hw.ncpu)
(cd build-native && make -j $NPROC)
#(cd build-native && make VERBOSE=1 -j $NPROC)
(build-native/test/cpp/cpptest 10)