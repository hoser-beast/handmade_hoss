@echo off

rem /Zi = debug

mkdir ..\..\build
pushd ..\..\build

cl /Zi ..\src\win32_handmade.cpp user32.lib

popd
