@echo off

set CURDIR=%CD%

set I_CC=icx

:: set INTEL_CC variable to use a different compiler (for example set INTEL_CC=icl )
if x%INTEL_CC% == x goto skip_set_CC
  set I_CC=%INTEL_CC%
:skip_set_CC

set scriptdir=%~dp0
cd %scriptdir%\..\..\..\fds
set fdsrepo=%CD%

cd %CURDIR%

set COMPILER=gcc
set COMPILER2=g++

set MSCOMPILER=X86

IF "%1" NEQ "g" SET COMPILER=%I_CC%
IF "%1" NEQ "g" SET COMPILER2=%I_CC%

IF "%1" EQU "m" SET COMPILER=cl
IF "%1" EQU "m" SET COMPILER2=cl

IF "%1" EQU "c" SET COMPILER=clang-cl
IF "%1" EQU "c" SET COMPILER2=clang-cl

SET COPYLIB=0
IF "%2" EQU "-copy" SET COPYLIB=1
IF "%2" EQU "-copy" SET FROMLIB=%3
IF "%2" EQU "-copy" SET TOLIB=%4

SET MSCOMPILER=x86_amd64

IF "%COMPILER%" NEQ "cl" GOTO MSenvexist

IF "%VS_COMPILER% NEQ "" GOTO VS_COMPILER_DEFINED
echo "*** Error: The environment variable, VS_COMPILER, pointing"
echo "           to the Visual Studio C compiler, cl, is not defined"
echo "*** compilation aborted
exit
:VS_COMPILER_DEFINED

IF "%MSCOMPILERS_DEFINED%" EQU "1" GOTO MSenvexist
echo Setting up Visual Studio compiler environment
set MSCOMPILERS_DEFINED=1
call "%VS_COMPILER%\..\vcvarsall" %MSCOMPILER%
GOTO Ienvexist
:MSenvexist

IF "%COMPILER%" NEQ "%I_CC%" GOTO Ienvexist
call %fdsrepo%\Build\scripts\setup_intel_compilers.bat >Nul
:Ienvexist
