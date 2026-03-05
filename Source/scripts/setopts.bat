@echo off
set COMPILER=icx
set COMPILER2=icx
set LIB=lib

if "%1" EQU "g" set COMPILER=gcc
if "%1" EQU "g" set COMPILER2=g++
if "%1" EQU "g" set LIB=lib

if "%1" EQU "c" set COMPILER=clang-cl
if "%1" EQU "c" set COMPILER2=clang-cl
if "%1" EQU "c" set LIB=llvm-lib
