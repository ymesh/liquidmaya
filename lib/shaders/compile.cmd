rem @echo off
rem 3Delight
for %%i in (src\*.sl) do shaderdl %%i
rem AIR
for %%i in (src\*.sl) do shaded %%i
rem Aqsis
for %%i in (src\*.sl) do aqsl %%i
rem Pixie
for %%i in (src\*.sl) do sdrc %%i
rem PRMan
for %%i in (src\*.sl) do shader -back %%i
rem RenderDotC
for %%i in (src\*.sl) do shaderdc %%i
del *.i
del *.cpp
del *.obj
del *.exp
del *.lib