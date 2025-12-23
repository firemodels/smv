@echo off
if x%FIREMODELS% == x set FIREMODELS=%userprofile%\FireModels_fork
%FIREMODELS%\fds\Build\impi_intel_win_dv\fds_impi_win_dv %* 
