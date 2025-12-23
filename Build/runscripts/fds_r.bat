@echo off
if x%FIREMODELS% == x set FIREMODELS=%userprofile%\FireModels_fork
%FIREMODELS%\fds\Build\impi_intel_win\fds_impi_win %*