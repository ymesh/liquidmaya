#!/bin/bash
tmpDir="tmp"
if [ -d $tmpDir ]; then
   rm -rf $tmpDir
fi
mkdir -p $tmpDir
cd $tmpDir


#export LIQUIDHOME=/tools/maya/liquidmaya
# GEN="NMake Makefiles"
# export ARCH=
export ARCH=-x64

#export LIQRMAN=generic
export LIQRMAN=PRMan
#export LIQRMAN=3Delight
# LIQRMAN=pixie
# LIQRMAN=aqsis
# LIQRMAN=air

function build_drivers(){

	echo "Building $LIQRMAN Liquid display driver"
	#echo using %GEN% ...

	cmake -D CMAKE_INSTALL_PREFIX:PATH=$LIQUIDHOME/displayDrivers $LIQUIDHOME/src/displayDrivers

	make
	make install

	cd ..

}

case $LIQRMAN in
	generic)
		export USE_RIBLIB=yes
		export LIQRMANPATH=$LIQUIDHOME/ribLib
		export LIQRMANFLAGS=-DGENERIC_RIBLIB
		export LIQDPYNAME=d_liqmaya.so
		export LIQDPYSRC=liqMayaDisplayDriver.cpp
		export LIQRMANLIBS=rib$ARCH
		export LIQRMANINC=$LIQRMANPATH
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_drivers
		;;
		
	air)
		export LIQRMANPATH=$AIRHOME
		export LIQRMANFLAGS=-DAIR
		export LIQDPYNAME=d_liqmaya.so
		export LIQDPYSRC=liqMayaDisplayDriverAir.cpp
		export LIQRMANLIBS=
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_drivers
		;;

	aqsis)
		export LIQRMANPATH=$AQSISHOME
		export LIQRMANFLAGS=-DAQSIS
		export LIQDPYNAME=liqmaya.so
		export LIQDPYSRC=liqMayaDisplayDriverAqsis.cpp
		export LIQRMANLIBS=ri2rib.lib
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_drivers
		;;

	pixie)
		export LIQRMANPATH=$PIXIEHOME
		export LIQRMANFLAGS=-DPIXIE
		export LIQDPYNAME=liqmaya.so
		export LIQDPYSRC=liqMayaDisplayDriverPixie.cpp
		export LIQRMANLIBS=ri.lib
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_drivers
		;;

	3Delight)
		export LIQRMANPATH=$DELIGHT
		export LIQRMANFLAGS=-DDELIGHT
		export LIQDPYNAME=liqmaya
		export LIQDPYSUFFIX=.dpy
		export LIQDPYSRC=liqMayaDisplayDriver3Delight.cpp
		export LIQRMANLIBS=3delight
		export LIBS=
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$LIQRMANPATH/lib
		build_drivers
		;;

	PRMan)
		export LIQRMANPATH=$RMANTREE
		export LIQRMANFLAGS=-DPRMAN
		export LIQDPYNAME=d_liqmaya
		export LIQDPYSUFFIX=.so
		export LIQDPYSRC=liqMayaDisplayDriver.cpp
		#export LIQRMANLIBS=prmansdk
    export LIQRMANLIBS=prman
		export LIBS=
		export LIQRMANINC=$LIQRMANPATH/include
		export LIQRMANLIB=$RMANTREE/lib
		build_drivers
		;;
esac




exit 0
