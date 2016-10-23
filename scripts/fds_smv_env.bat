@echo off

:: ---- official FDS and smokeview version strings
set fds_version=6.5.3
set smv_version=6.4.1

:: ---- test FDS and smokeview version strings
set smv_revision=SMV6.4.1-16-g975c1cf
set fds_revision=02a6097

:: ---- for obtaining log entries
set smvlogdate="7-Oct-2016"

:: PC repo

set svn_root=%userprofile%\FireModels_fork
set fdswikirepo=%svn_root%\wikis
set svn_drive=c:

:: Linux/OSX repo

set linux_svn_root=FireModels_fork

:: firebot/smokebot repos

set firebotrepo=/home4/firebot/FireModels_central
set smokebotrepo=/home4/smokebot/FireModels_central

:: Linux user and host name

set linux_hostname=burn.el.nist.gov
set linux_username=%username%
set linux_logon=%linux_username%@%linux_hostname%

:: OSX user and host name

set osx_hostname=bluesky.el.nist.gov
set osx_username=%username%
set osx_logon=%osx_username%@%osx_hostname%

:: FDS/Smokeview version

set fdssmv_major_version=6
set fds_edition=FDS6
set smv_edition=SMV6
