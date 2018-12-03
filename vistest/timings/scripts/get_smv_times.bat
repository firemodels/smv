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
call :get_time %SMOKEVIEW1% plume_timing_startup.ssf plume_timing  time1
call :get_time %SMOKEVIEW1% plume_timing_iso.ssf     plume_timing  time2
call :get_time %SMOKEVIEW1% plume_timing_3dsmoke.ssf plume_timing  time3
call :get_time %SMOKEVIEW1% plume_timing_slice.ssf   plume_timing  time4
call :get_time %SMOKEVIEW1% plume_timing_vslice.ssf  plume_timing  time5
echo %time1%,%time2%,%time3%,%time4%,%time5%

::------------

call :get_time %SMOKEVIEW2% plume_timing_startup.ssf plume_timing  time1
call :get_time %SMOKEVIEW2% plume_timing_iso.ssf     plume_timing  time2
call :get_time %SMOKEVIEW2% plume_timing_3dsmoke.ssf plume_timing  time3
call :get_time %SMOKEVIEW2% plume_timing_slice.ssf   plume_timing  time4
call :get_time %SMOKEVIEW2% plume_timing_vslice.ssf  plume_timing  time5
echo %time1%,%time2%,%time3%,%time4%,%time5%

goto eof

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


