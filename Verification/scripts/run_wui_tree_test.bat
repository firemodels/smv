@echo off
set git_drive=d:

set GITROOT=%CD%\..\..\
set WUIDIR=%GITROOT%\Verification\Wui

set FDS=%GITROOT%\fds\Build\intel_win_64\fds_win_64

echo %FDS%
cd %WUIDIR%
%FDS% onetree_surf_1mesh.fds
smokezip -part2iso onetree_surf_1mesh
pause
