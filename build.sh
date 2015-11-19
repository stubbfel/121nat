#!/bin/bash
cd lib
#sh buildalllibs.sh
cd ..
mkdir -p build/121Nat
cd build/121Nat
cmake ../../
make
cp 121Nat ../
cd ../..
rm -Rf build/121Nat
