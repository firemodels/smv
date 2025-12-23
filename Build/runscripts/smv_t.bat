@echo off
if x%FIREMODELS% == x set FIREMODELS=%userprofile%\FireModels_fork
%FIREMODELS%\smv\Build\smokeview\intel_win\smokeview_win_test.exe %* 
