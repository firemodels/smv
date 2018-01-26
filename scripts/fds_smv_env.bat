@echo off

:: ---- FDS and smokeview version ----

set fds_version=6.6.0test
set smv_version=6.6.1test

:: ---- FDS and smokeview revision ----

set smv_revision=unknown
set fds_revision=unknown
if exist %userprofile%\smv_revision.txt (
  set /p smv_revision=<%userprofile%\smv_revision.txt
)
if exist %userprofile%\fds_revision.txt (
  set /p fds_revision=<%userprofile%\fds_revision.txt
)

:: ---- log entry date ----

set smvlogdate="18-Jan-2017"

:: ---- repo locations ----

::*** PC
set svn_root=%userprofile%\FireModels_fork
set fdswikirepo=%svn_root%\wikis
set svn_drive=c:

::*** Linux/OSX
set linux_svn_root=FireModels_fork
set compiler_dir=fire-notes/INSTALL/LINUX/INTEL_17u4
set misc_dir=fire-notes/INSTALL/LIBS/LINUX/LIB64

:: ---- MPI library locations ----

:: set to INTEL if using Intel MPI library
set linux_mpi_version=INTEL
set osx_mpi_version=3.0.0
::set osx_mpi_version=1.8.4

:: ---- bot locations ----

set firebotrepo=/home2/smokevis2/firebot/FireModels_central
set firebothome=/home2/smokevis2/firebot

set smokebotrepo=/home2/smokevis2/smokebot/FireModels_central
set smokebothome=/home2/smokevis2/smokebot

:: ---- hostnames ----

::*** linux
set linux_hostname=burn.el.nist.gov
set linux_username=%username%
set linux_logon=%linux_username%@%linux_hostname%

::*** OSX
set osx_hostname=ignis.el.nist.gov
::set osx_hostname=192.168.1.13
set osx_username=%username%
set osx_logon=%osx_username%@%osx_hostname%
