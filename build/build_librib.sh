#!/bin/bash

tmpDir="tmp"
if [ -d $tmpDir ]; then
   rm -rf $tmpDir
fi
mkdir -p $tmpDir
cd $tmpDir

# export LIQUIDHOME=$LIQUIDCODE
# ZLIB_LIBRARIES=C:/tools/zlib/lib
# ZLIB_INCLUDE_DIRS=C:/tools/zlib/include

# GEN=
#export ARCH=
export ARCH=-x64

echo "Building ribLib for Liquid $ARCH" 
# echo using %GEN% ...

cmake -D CMAKE_INSTALL_PREFIX:PATH=$LIQUIDHOME/ribLib $LIQUIDHOME/ribLib

make
make install

cd ..
