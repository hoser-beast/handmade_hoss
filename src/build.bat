@echo off

rem /FAsc = full path to file
rem /Zi = debug

mkdir ..\..\build
pushd ..\..\build

cl /FAsc /Zi ..\src\win32_handmade.cpp user32.lib gdi32.lib

popd
