@echo off

set CURDIR=%CD%
cd ..
set ROOT=%CD%
cd %CURDIR%
set MERGEPO=%ROOT%\Build\mergepo\intel_win_64\mergepo_win_64

for %%a in ( smokeview_de.po smokeview_fr.po ) do (
  echo updating %%a 
  %MERGEPO% -c %%a smokeview_template.po > %%a.revised
)
