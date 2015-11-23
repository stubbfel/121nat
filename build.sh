#!/bin/bash
cd lib
sh buildalllibs.sh
cd ..
mkdir -p build/121Natbuild
cd build/121Natbuild
cmake ../../
make
cp src/121Nat ../
cd ../..
rm -Rf build/121Natbuild
