@echo off
set CURDIR=%CD%
set SMOKEVIEW1=smv654u4
set SMOKEVIEW2=smv671u696
set CASEDIR=%CURDIR%\..\cases
set CASE=plume_timing
::set CASE=isotest1

cd ..\..\..\
set smvrepo=%CD%

set TIME=%smvrepo%\Build\cputime\intel_win_64\cputime_64.exe
set FLUSH=%smvrepo%\Build\flush\intel_win_64\flush_win_64.exe

cd %CASEDIR%
call :run_cases %SMOKEVIEW1% %CASE%

call :run_cases %SMOKEVIEW2% %CASE%

goto eof

:: -------------------------------------------------------------
 :run_cases
:: -------------------------------------------------------------
set SMV=%1
set smvcase=%2

call :get_time %SMV% %smvcase%startup.ssf %smvcase%  time1
call :get_time %SMV% %smvcase%iso.ssf     %smvcase%  time2
call :get_time %SMV% %smvcase%3dsmoke.ssf %smvcase%  time3
call :get_time %SMV% %smvcase%slice.ssf   %smvcase%  time4
call :get_time %SMV% %smvcase%vslice.ssf  %smvcase%  time5
echo %time1%,%time2%,%time3%,%time4%,%time5%

exit /b 0

:: -------------------------------------------------------------
 :get_time
:: -------------------------------------------------------------

set smv=%1
set script=%2
set case=%3
set timevar=%4

%FLUSH% 1> Nul 2>&1
%TIME% %smv% -scriptfile %script% %case%  1> tmp.out 2>&1
tail -1 tmp.out | gawk "{print $1}" > tmp2.out
set /p %timevar%=<tmp2.out
erase tmp.out

exit /b 0

:eof
cd %CURDIR%


