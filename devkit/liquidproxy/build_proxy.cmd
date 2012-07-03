@echo off 

SET tmpdir=tmp
if exist %tmpdir% rmdir /s /q %tmpdir%
mkdir %tmpdir%
cd %tmpdir%

SET LIQUIDHOME=c:/tools/liquidmaya
SET VSINSTALLDIR="C:\Program Files (x86)\Microsoft Visual Studio 8"

SET GEN="NMake Makefiles"
rem SET GEN="Visual Studio 8 2005"
rem SET GEN="Visual Studio 8 2005 Win64"
SET ZLIB_INCLUDE_DIRS=C:/tools/zlib/include
SET BOOST_ROOT=C:/boost/boost_1_41_0

:set_maya_arch
SET ARCH=
rem SET ARCH=-x64

IF DEFINED ARCH (echo ARCH defined) ELSE ( goto set_x32 )
IF %ARCH% == -x64 GOTO set_x64 

:set_x32
@echo Setup VC variables for x86 mode
SET ZLIB_LIBRARIES=C:/tools/zlib/lib/zlib.lib
call %VSINSTALLDIR%\VC\vcvarsall.bat x86
goto build

:set_x64
SET ZLIB_LIBRARIES=C:/tools/zlib64/static_x64/zlibstat.lib
call %VSINSTALLDIR%\VC\vcvarsall.bat amd64
goto build


:build
rem if exist override.cmd call override.cmd
echo Building liquidproxy for Liquid %ARCH% 
echo using %GEN% ...
cmake -G %GEN% -D CMAKE_INSTALL_PREFIX:PATH=c:/tools/liquidmaya/devkit/liquidproxy/ c:/tools/liquidmaya/devkit/liquidproxy/src
nmake
nmake install
cd ..

