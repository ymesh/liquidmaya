#!/bin/bash
tmpDir="tmp"
#if [ -d $tmpDir ]; then
#   rm -rf $tmpDir
#   echo "$tmpDir"
#fi
#mkdir -p $tmpDir
cd $tmpDir

#export LIQUIDHOME=$LIQUIDCODE

# Set Maya version
#export MAYA_VERSION=2008
#export MAYA_VERSION=2009
#export MAYA_VERSION=2010
#export MAYA_VERSION=2011
export MAYA_VERSION=2012

# Set Maya architecture
#export ARCH=
export ARCH=-x64

OS_NAME=`uname`
if [ $OS_NAME == Darwin ]; then
	export LIQPLUGSUFFIX=.bundle
	export AW_LOCATION=/Applications/Autodesk
	export BOOST_ROOT=~/tools/boost/boost_1_41_0
	OS=osx
else
	export ARCH=-x64
	export LIQPLUGSUFFIX=.so
	export AW_LOCATION=/usr/autodesk
	export BOOST_ROOT=/usr/include
	OS=linux
fi

export MAYALIBS="-lFoundation -lOpenMaya -lOpenMayaRender -lOpenMayaUI -lOpenMayaAnim -lOpenMayaFX -lGLU -lGL"
export LIBS=-lm

#SET ZLIB_LIBRARIES=C:/tools/zlib/lib
#SET ZLIB_INCLUDE_DIRS=C:/tools/zlib/include

function build_liquid(){
	echo "Building $LIQRMAN Liquid for Maya $MAYA_VERSION$ARCH"
	#echo using %GEN% ...
	export MAYA_LOCATION=$AW_LOCATION/maya$MAYA_VERSION$ARCH
	cmake -DCMAKE_INSTALL_PREFIX:PATH=$LIQUIDHOME/bin/Maya$MAYA_VERSION$ARCH -DMAYA_LOCATION=$MAYA_LOCATION $LIQUIDHOME/src
	make
	make install
	cd ..
}

#export LIQRMAN=generic
export LIQRMAN=PRMan
# LIQRMAN=3Delight
# LIQRMAN=pixie
# LIQRMAN=aqsis
# LIQRMAN=air

LIQUIDSHORTVERSION=`tr -d \"\\"\\"\" < $LIQUIDHOME/src/liquid.version`
BUILDDATE=`date '+%d. %b. %Y : %R'`
export LIQUIDVERSION=" $LIQUIDSHORTVERSION for $LIQRMAN RiLib, $BUILDDATE "


case $LIQRMAN in
	generic)
		export USE_RIBLIB=yes
		export LIQRMANPATH=$LIQUIDHOME/ribLib
		export LIQRMANFLAGS="-DGENERIC_RIBLIB -DNO_RICMD" 
		export LIQRMANLIBS=rib$ARCH
		export LIQRMANINC=$LIQRMANPATH
		export LIQRMANLIB=$LIQRMANPATH/lib/$OS
		build_liquid
		;;

	air)
		export LIQRMANPATH=$AIRHOME
		export LIQRMANFLAGS=-DAIR
		export LIQRMANLIBS=
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_liquid
		;;

	aqsis)
		export LIQRMANPATH=$AQSISHOME
		export LIQRMANFLAGS=-DAQSIS
		export LIQRMANLIBS=ri2rib.lib
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_liquid
		;;

	pixie)
		export LIQRMANPATH=$PIXIEHOME
		export LIQRMANFLAGS=-DPIXIE
		export LIQRMANLIBS=ri.lib
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_liquid
		;;

	3Delight)
		export LIQRMANPATH=$DELIGHT
		export LIQRMANFLAGS=-DDELIGHT
		export LIQRMANLIBS=3delight
		export LIBS=
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_liquid
		;;

	PRMan)
		export LIQRMANPATH=$RMANTREE
		export LIQRMANFLAGS=-DPRMAN
		export LIQRMANLIBS=prman
		export LIBS=
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$RMANTREE/lib
		build_liquid
		;;
esac

exit 0

