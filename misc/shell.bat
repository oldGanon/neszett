@echo off

REM MSVC
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64

REM LLVM
set path="C:\Program Files\LLVM\bin";%path%

REM SCRIPTS
set path=%~dp0;%path%

cls