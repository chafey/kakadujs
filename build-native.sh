#!/bin/sh
#rm -rf build-native
mkdir -p build-native
#(cd build-native && CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug ..)
(cd build-native && CC=clang CXX=clang++ cmake ..)
(cd build-native && make VERBOSE=1 -j 8)
(build-native/test/cpp/cpptest 20)