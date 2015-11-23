#!/bin/bash
mkdir -p bin
mkdir -p build/jsoncpp/
cd  build/jsoncpp/
cmake ../../src/jsoncpp
make
cp src/lib_json/libjsoncpp.a ../../bin 
cd ../..
rm -Rf build/jsoncpp
