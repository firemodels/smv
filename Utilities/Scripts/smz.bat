@echo off
set reporoot=%userprofile%\FireModels_fork
set smzbin=%reporoot%\smv\Build\smokezip\intel_win_64
%smzbin%\smokezip_win_64 %*
