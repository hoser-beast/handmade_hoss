@echo off
clang-format -i --style=file:clang-format.txt *.cpp
clang-format -i --style=file:clang-format.txt *.h
