@echo off 

SET tmpdir=tmp
if exist %tmpdir% rmdir /s /q %tmpdir%
mkdir %tmpdir%
cd %tmpdir%

rem SET LIQUIDHOME=d:/code/maya/liquidmaya
SET VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 9.0"
rem SET VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 10.0"

SET GEN="NMake Makefiles"
rem SET GEN="Visual Studio 8 2005"
rem SET GEN="Visual Studio 8 2005 Win64"

SET ZLIB_INCLUDE_DIRS=d:/code/LIBS/zlib/include

SET AW_LOCATION=c:/Autodesk
:set_maya_version
rem SET MAYA_VERSION=2008
rem SET MAYA_VERSION=2009
rem SET MAYA_VERSION=2010
rem SET MAYA_VERSION=2011
SET MAYA_VERSION=2012

:set_maya_arch
rem SET ARCH=
SET ARCH=-x64


IF DEFINED ARCH (echo ARCH defined) ELSE ( goto set_x32 )
IF %ARCH% == -x64 GOTO set_x64 

:set_x32
@echo Setup VC variables for x86 mode
rem SET ZLIB_LIBRARIES=d:/code/LIBS/zlib/lib/Win32/zlibwapi.lib
call %VSINSTALLDIR%\VC\vcvarsall.bat x86
goto build

:set_x64
rem SET ZLIB_LIBRARIES=d:/code/LIBS/zlib/lib/x64/zlibwapi.lib
call %VSINSTALLDIR%\VC\vcvarsall.bat amd64
goto build


:build
rem if exist override.cmd call override.cmd
echo Building ribLib for Liquid %ARCH% 

SET MAYA_LOCATION=%AW_LOCATION%/Maya%MAYA_VERSION%%ARCH%
SET ZLIB_LIBRARIES=%MAYA_LOCATION%/lib/libzlib.lib

echo using %GEN% ...
cmake -G %GEN% -D CMAKE_INSTALL_PREFIX:PATH=%LIQUIDHOME%/ribLib %LIQUIDHOME%/ribLib
nmake
nmake install
cd ..

