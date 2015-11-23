#!/bin/bash
mkdir -p  temp/121Nat/lib

cp -R ../../src/ temp/121Nat/
cp -R ../../lib/src/ temp/121Nat/lib/
cp ../../lib/*.sh temp/121Nat/lib/
cp ../../build.sh temp/121Nat/
cp ../../CMakeLists.txt temp/121Nat/
cp PKGBUILD.example temp/PKGBUILD

cd temp
tar -cvzf 121Nat-1.0.tar.gz 121Nat/
md5=($(md5sum 121Nat-1.0.tar.gz))
echo "md5sums=('$md5')" >> PKGBUILD
makepkg
cp *.pkg.tar.xz ../
cp *.tar.gz ../
cp -f PKGBUILD ../PKGBUILD
cd ..
rm -Rf temp
