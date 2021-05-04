@echo off

:: ---- FDS and smokeview version ----

set fds_version=FDS6.7.5
set fds_tag=FDS6.7.5
set fds_versionbase=6.7.5

set smv_version=SMV6.7.15
set smv_tag=SMV6.7.15
set smv_versionbase=6.7.15

set fds_edition=FDS6
set fds_debug=0

set fds_tag_name=release
set smv_tag_name=release

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

set smvlogdate="3-May-2021"

:: ---- repo locations ----

::*** PC
set svn_root=%userprofile%\FireModels_fork
set fdswikirepo=%svn_root%\wikis
set svn_drive=c:

::*** Linux/OSX
set userhome=/home/gforney
set bot_home=/home4/gforney
set linux_svn_root=FireModels_fork
set compiler_dir=fire-notes/INSTALL/LINUX/INTEL_19u1
set misc_dir=fire-notes/INSTALL/LIBS/LINUX/LIB64

:: ---- MPI library locations ----

:: set to INTEL if using Intel MPI library
set linux_mpi_version=INTEL
set linux_intel_mpi_version=oneapi


set osx_mpi_version=4.1.0
set osx_openmpi_dir=/opt/openmpi410_oneapi_64
::set osx_mpi_version=1.8.4

set linux_comp_version=20u1
set osx_comp_version=oneapi

:: ---- INTEL compiler version ----

set INTELVERSION=INTELoneapi

:: ---- Guide locations ----

set GUIDE_DIR=.bundle/pubs

:: ---- openmpi library locations ----

set OPENMPI_DIR=.bundle/OPENMPI

:: ---- bundle locations ----

::set BUNDLE_DIR=.bundle/BUNDLE

:: ---- bot locations ----

set firebotrepo=/home2/smokevis2/firebot/FireModels_clone
set firebothome=/home2/smokevis2/firebot

set smokebotrepo=/home2/smokevis2/smokebot/FireModels_central
set smokebothome=/home2/smokevis2/smokebot

:: plink options
set plink_options=-no-antispoof

:: ---- hostnames ----

::*** linux
set linux_hostname=blaze.el.nist.gov
::set linux_hostname=192.168.1.14
set linux_username=%username%
set linux_logon=%linux_username%@%linux_hostname%

::*** OSX
set osx_hostname=bevo.el.nist.gov
set osx_hostname=192.168.1.146
set osx_username=%username%
set osx_logon=%osx_username%@%osx_hostname%
