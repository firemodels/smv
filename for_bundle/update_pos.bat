@echo off

set CURDIR=%CD%
cd ..
set ROOT=%CD%
cd %CURDIR%
set MERGEPODIR=%ROOT%\Build\mergepo\intel_win_64
set MERGEPO=%MERGEPODIR%\mergepo_win_64

for %%a in ( smokeview_de.po smokeview_fr.pl ) do (
  echo updating %%a 
  %MERGEPO% -c %%a smokeview_template.po > %%a.revised
)
