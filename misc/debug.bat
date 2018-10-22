@echo off

cd %~dp0
cd ..

pushd build
devenv neszett.exe
popd build