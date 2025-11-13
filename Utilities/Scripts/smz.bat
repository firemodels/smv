@echo off
set reporoot=%userprofile%\FireModels_fork
set smzbin=%reporoot%\smv\Build\smokezip\intel_win
%smzbin%\smokezip_win %*
