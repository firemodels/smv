@echo off

:: ---- smokeview revision ----

set smv_revision=unknown
set fds_revision=unknown
if exist %userprofile%\.bundle\smv_revision.txt (
  set /p smv_revision=<%userprofile%\.bundle\smv_revision.txt
)

:: ---- log entry date ----

set smvlogdate="15-September-2025"

:: ---- repo locations ----

::*** PC
if "x%git_root%" NEQ "x" goto skipgit
set git_root=%userprofile%\FireModels_fork
:skipgit

set git_drive=c:

::*** linux/osx - note: osx root uses linux_git_root variable
set linux_git_root=FireModels_fork

:: ---- MPI library locations ----

:: set to INTEL if using Intel MPI library
set linux_mpi_version=INTEL
set linux_intel_mpi_version=oneapi

set osx_mpi_version=4.1.0
set osx_comp_version=oneapi

:: ---- INTEL compiler version ----

set INTELVERSION=INTELoneapi1p6

:: plink options
set plink_options=-no-antispoof

:: ---- hostnames ----

::*** linux
set linux_hostname=spark.nist.gov
set linux_username=%username%
set linux_logon=%linux_username%@%linux_hostname%

::*** OSX
set osx_hostname=excess.el.nist.gov
set osx_username=%username%
set osx_logon=%osx_username%@%osx_hostname%
