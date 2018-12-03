@echo off
set CURDIR=%CD%
set SMOKEVIEW1=smv654u4
set SMOKEVIEW2=smv671u696
set LABEL=smokeview
set CASE=plume_timing
set SCRIPT=plume_timing_\ 
set CASEDIR=%CURDIR%\..\cases
set score=_

cd ..\..\..\
set smvrepo=%CD%
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

flush_mem 1> Nul 2>&1
ptime %smv% -scriptfile %script% %case%  1> tmp.out 2>&1
tail -1 tmp.out | gawk "{print $3}" > tmp2.out
set /p %timevar%=<tmp2.out
rm tmp.out tmp2.out

exit /b 0

:eof
cd %CURDIR%


