#!/bin/bash

# this script does a clean build and is useful for development when modifying cmakefiles
clear
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release 
#(cd build && make VERBOSE=1 -j) || { exit 1; }
(cd build && make -j) || { exit 1; }
build/test/cpp/cpptest
