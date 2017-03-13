@echo off

:: ---- version: official FDS and smokeview ----
set fds_version=6.5.3
set smv_version=6.4.4

:: ---- revision: test FDS and smokeview ----
set smv_revision=unknown
set fds_revision=unknown
if exist %userprofile%\smv_revision.txt (
  set /p smv_revision=<%userprofile%\smv_revision.txt
)
if exist %userprofile%\fds_revision.txt (
  set /p fds_revision=<%userprofile%\fds_revision.txt
)

:: ---- log entry date ----
set smvlogdate="23-Dec-2016"

:: ---- repo locations ----

::*** PC
set svn_root=%userprofile%\FireModels_fork
set fdswikirepo=%svn_root%\wikis
set svn_drive=c:

::*** Linux/OSX
set linux_svn_root=FireModels_fork
set compiler_dir=fire-notes/INSTALL/LIBS/LINUX/LIB64_i17
set misc_dir=fire-notes/INSTALL/LIBS/LINUX/LIB64

::*** firebot/smokebot
set firebotrepo=/home4/firebot/FireModels_central
set smokebotrepo=/home4/smokebot/FireModels_central

:: ---- hostnames ----

::*** linux
set linux_hostname=burn.el.nist.gov
set linux_username=%username%
set linux_logon=%linux_username%@%linux_hostname%

::*** OSX
set osx_hostname=ignis.el.nist.gov
set osx_username=%username%
set osx_logon=%osx_username%@%osx_hostname%

:: ---- FDS/Smokeview version ----
set fdssmv_major_version=6
set fds_edition=FDS6
set smv_edition=SMV6
