@echo off

:: ---- FDS and smokeview version ----

set fds_version=FDS6.7.0
set smv_version=SMV6.7.1
set fds_debug=0
set fds_edition=FDS6

:: ---- FDS and smokeview revision ----

set smv_revision=unknown
set fds_revision=unknown
if exist %userprofile%\.bundle\smv_revision.txt (
  set /p smv_revision=<%userprofile%\.bundle\smv_revision.txt
)
if exist %userprofile%\.bundle\fds_revision.txt (
  set /p fds_revision=<%userprofile%\.bundle\fds_revision.txt
)

:: ---- log entry date ----

set smvlogdate="19-Jun-2018"

:: ---- repo locations ----

::*** PC
set svn_root=%userprofile%\FireModels_fork
set fdswikirepo=%svn_root%\wikis
set svn_drive=c:

::*** Linux/OSX
set linux_svn_root=FireModels_fork

:: ---- MPI version ----

:: set to INTEL if using Intel MPI library

set linux_mpi_version=INTEL
set osx_mpi_version=3.0.0

:: ---- lib/bin locations ----

set INTEL_LIB_DIR=/var/local/bundle/INTEL/INTEL_18u2/LIB
set INTEL_BIN_DIR=/var/local/bundle/INTEL/INTEL_18u2/bin64
set OS_LIB_DIR=/var/local/bundle/OSLIBS/LINUX

:: ---- Guide locations ----

set GUIDE_DIR=.bundle/pubs

:: ---- openmpi library locations ----

set OPENMPI_DIR=.bundle/OPENMPI

:: ---- bundle locations ----

set BUNDLE_DIR=.bundle/BUNDLE

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
set osx_hostname=floga.el.nist.gov
::set osx_hostname=192.168.1.13
set osx_username=%username%
set osx_logon=%osx_username%@%osx_hostname%
