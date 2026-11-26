@echo off
set COMPILER=icx
set COMPILER2=icx
rem Keep the archive tool separate from the linker's LIB search path.
set SMV_LIB_TOOL=lib

if "%1" EQU "g" set COMPILER=gcc
if "%1" EQU "g" set COMPILER2=g++

if "%1" EQU "c" set COMPILER=clang-cl
if "%1" EQU "c" set COMPILER2=clang-cl
if "%1" EQU "c" set SMV_LIB_TOOL=llvm-lib
