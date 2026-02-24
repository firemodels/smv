@echo off
if defined ONEAPI_ROOT exit /b
if not defined ONEAPIDIR set "ONEAPIDIR=C:\Program Files (x86)\Intel\oneAPI"
IF EXIST "%ONEAPIDIR%\setvars.bat" echo *** setting up Intel oneapi compiler environment
IF EXIST "%ONEAPIDIR%\setvars.bat" call "%ONEAPIDIR%\setvars" intel64>Nul
if not defined ONEAPI_ROOT echo ***warning: oneapi compiler setup failed 
