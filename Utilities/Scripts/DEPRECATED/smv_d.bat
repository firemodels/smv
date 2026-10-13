@echo off
set reporoot=%userprofile%\FireModels_fork
set smvbin=%reporoot%\smv\Build\smokeview\intel_win
set bindir=%reporoot%\smv\Build\for_bundle
%smvbin%\smokeview_win_test_db -bindir %bindir% %*
