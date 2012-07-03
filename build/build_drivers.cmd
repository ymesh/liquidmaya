@echo off 

SET tmpdir=tmp
if exist %tmpdir% rmdir /s /q %tmpdir%
mkdir %tmpdir%
cd %tmpdir%

SET LIQUIDHOME=d:/code/maya/liquidmaya

rem SET VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 8"
SET VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 9.0"
rem SET VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 10.0"
SET GEN="NMake Makefiles"

rem SET ARCH=
rem call %VSINSTALLDIR%\VC\vcvarsall.bat x86

SET ARCH=-x64
call %VSINSTALLDIR%\VC\vcvarsall.bat amd64

rem SET LIQRMAN=generic
SET LIQRMAN=PRMan
rem SET LIQRMAN=3delight
rem SET LIQRMAN=pixie
rem SET LIQRMAN=aqsis
rem SET LIQRMAN=air

rem if /i %LIQRMAN% == generic  goto generic

GOTO %LIQRMAN%

:generic
SET USE_RIBLIB=yes
SET LIQRMANPATH=%LIQUIDHOME%/ribLib
SET LIQRMANFLAGS=-DGENERIC_RIBLIB
SET LIQDPYNAME=	d_liqmaya.so
SET LIQDPYSRC=liqMayaDisplayDriver.cpp
SET LIQRMANLIBS=librib.lib
SET LIQRMANINC=%LIQRMANPATH%
SET LIQRMANLIB=%LIQRMANPATH%/lib
GOTO build_drivers

:air
SET LIQRMANPATH=%AIRHOME%
SET LIQRMANFLAGS=	-DAIR
SET LIQDPYNAME=	d_liqmaya.so
SET LIQDPYSRC=liqMayaDisplayDriverAir.cpp
SET LIQRMANLIBS=
SET LIQRMANINC=%LIQRMANPATH%/include
SET LIQRMANLIB=%LIQRMANPATH%/lib
GOTO build_drivers

:aqsis
SET LIQRMANPATH=%AQSISHOME%
SET LIQRMANFLAGS=	-DAQSIS
SET LIQDPYNAME=	liqmaya.so
SET LIQDPYSRC=liqMayaDisplayDriverAqsis.cpp
SET LIQRMANLIBS=ri2rib.lib
SET LIQRMANINC=%LIQRMANPATH%/include
SET LIQRMANLIB=%LIQRMANPATH%/lib
GOTO build_drivers

:pixie
SET LIQRMANPATH=%PIXIEHOME%
SET LIQRMANFLAGS=	-DPIXIE
SET LIQDPYNAME=liqmaya.so
SET LIQDPYSRC=liqMayaDisplayDriverPixie.cpp
SET LIQRMANLIBS=ri.lib
SET LIQRMANINC=%LIQRMANPATH%/include
SET LIQRMANLIB=%LIQRMANPATH%/lib
GOTO build_drivers

:3delight
SET LIQRMANPATH=%DELIGHT%
SET LIQRMANFLAGS=	-DDELIGHT
SET LIQDPYNAME=liqmaya
SET LIQDPYSUFFIX=.dpy
SET LIQDPYSRC=liqMayaDisplayDriver3Delight.cpp
SET LIQRMANLIBS	=3delight.lib
SET LIBS=ws2_32.lib
SET LIQRMANINC=%LIQRMANPATH%/include
SET LIQRMANLIB=%LIQRMANPATH%/lib
GOTO build_drivers

:PRMan
SET LIQRMANPATH=%RMANTREE%
SET LIQRMANFLAGS=-DPRMAN
SET LIQDPYNAME=d_liqmaya
SET LIQDPYSUFFIX=.dll
SET LIQDPYSRC=liqMayaDisplayDriver.cpp
SET LIQRMANLIBS=libprman.lib
SET LIBS=ws2_32.lib
SET LIQRMANINC=%LIQRMANPATH%/include
SET LIQRMANLIB=%RMANTREE%/lib
GOTO build_drivers


:build_drivers
rem if exist override.cmd call override.cmd
echo Building "%LIQRMAN%" Liquid display driver
echo using %GEN% ...

cmake -G %GEN% -D CMAKE_INSTALL_PREFIX:PATH=%LIQUIDHOME%/displayDrivers %LIQUIDHOME%/src/displayDrivers

nmake
nmake install
cd ..

pause
