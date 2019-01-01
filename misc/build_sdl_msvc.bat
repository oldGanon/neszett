@echo off

REM -Od disable Optimization
REM -O2 Optimization [Release Mode]
set OptimizationFlag=-O2
set CompilerFlags=-c -MT -nologo -GS- -Gs9999999 -Gm- -GR- -EHa- -Oi -W4 -wd4201 -wd4100 -wd4189 -wd4505 -FC -Z7 %OptimizationFlag%
set NOCRT=-nodefaultlib kernel32.lib
set LinkerFlags=-nologo -subsystem:windows -stack:0x100000,0x100000 -debug -pdb:neszett.pdb -out:neszett.exe -map:neszett.map -opt:ref /LIBPATH:"..\lib\x64" %NOCRT% opengl32.lib SDL2.lib SDL2main.lib icon.res

cd %~dp0
cd ..

IF NOT EXIST build mkdir build
pushd build

IF EXIST icon.res goto skiprc
rc /r -nologo ..\misc\icon.rc
robocopy "..\misc" "..\build" icon.res /njh /njs
:skiprc

REM 64-bit build
REM ml64 -nologo -c -Fomath.obj ..\code\math\math.asm
cl %CompilerFlags% ..\code\main.cpp /I..\include
link %LinkerFlags% main.obj

robocopy "..\lib\x64" "..\build" SDL2.dll /njh /njs
popd