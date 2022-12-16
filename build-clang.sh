#!/bin/bash

# this script does a clean build and is useful for development when modifying cmakefiles

rm -rf build-clang
cmake -S . -B build-clang -DCMAKE_BUILD_TYPE=Release --preset=clang
(cd build-clang; make VERBOSE=1 -j)
build-clang/test/cpp/cpptest