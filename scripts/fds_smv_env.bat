@echo off

:: ---- smokeview revision ----

set smv_revision=unknown
set fds_revision=unknown
if exist %userprofile%\.bundle\smv_revision.txt (
  set /p smv_revision=<%userprofile%\.bundle\smv_revision.txt
)

:: ---- log entry date ----

set smvlogdate="18-July-2025"

:: ---- repo locations ----

::*** PC
if "x%git_root%" NEQ "x" goto skipgit
set git_root=%userprofile%\FireModels_fork
:skipgit

set fdswikirepo=%git_root%\wikis
set git_drive=c:

::*** Linux/OSX
set userhome=/home/gforney
set bot_home=/home4/gforney
set linux_git_root=FireModels_fork
set compiler_dir=fire-notes/INSTALL/LINUX/INTEL_19u1
set misc_dir=fire-notes/INSTALL/LIBS/LINUX/LIB64

:: ---- MPI library locations ----

:: set to INTEL if using Intel MPI library
set linux_mpi_version=INTEL
set linux_intel_mpi_version=oneapi


set osx_mpi_version=4.1.0
set osx_openmpi_dir=/opt/openmpi410_oneapi_64

set linux_comp_version=20u1
set osx_comp_version=oneapi

:: ---- INTEL compiler version ----

set INTELVERSION=INTELoneapi1p6

:: ---- Guide locations ----

set GUIDE_DIR=.bundle/pubs

:: ---- openmpi library locations ----

set OPENMPI_DIR=.bundle/OPENMPI

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
