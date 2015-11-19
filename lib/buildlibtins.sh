#!/bin/bash
mkdir -p bin
mkdir -p build/libtins/
cd  build/libtins/
cmake ../../src/libtins -DLIBTINS_BUILD_SHARED=1 -DLIBTINS_ENABLE_CXX11=1 -DLIBTINS_ENABLE_WPA2=0 -DLIBTINS_ENABLE_DOT11=0 -DHAVE_PCAP_IMMEDIATE_MODE=1
make
cp lib/libtins.so ../../bin 
cd ../..
rm -Rf build/libtins
