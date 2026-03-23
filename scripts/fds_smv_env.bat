@echo off

:: ---- log entry date ----

set smvlogdate="18-Mar-2026"

:: ---- hostnames ----

::*** linux
set linux_hostname=spark.nist.gov
set linux_username=%username%
set linux_logon=%linux_username%@%linux_hostname%

::*** OSX
set osx_hostname=wildfire.campus.nist.gov
set osx_username=%username%
set osx_logon=%osx_username%@%osx_hostname%

:: ------- should not need to change settings below -----------

:: ---- smokeview revision ----

set smv_revision=unknown
if exist %userprofile%\.bundle\smv_revision.txt set /p smv_revision=<%userprofile%\.bundle\smv_revision.txt

:: ---- repo locations ----

::*** PC
cd ..\..
set git_root=%CD%

::*** linux/osx - note: osx root uses linux_git_root variable
set linux_git_root=FireModels_fork
for %%a in ("%git_root%") do set linux_git_root=%%~nxa
