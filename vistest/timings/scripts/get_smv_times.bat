@echo off
set CURDIR=%CD%
set SMOKEVIEW1=smv654u4
set SMOKEVIEW2=smv671u696
set CASEDIR=%CURDIR%\..\cases
set CASE=plume_timing
::set CASE=isotest1

cd ..\..\..\
set smvrepo=%CD%

set TIME=%smvrepo%\Build\timep\intel_win_64\timep_win_64.exe
set FLUSH=%smvrepo%\Build\flush\intel_win_64\flush_win_64.exe -g 16 -w

:: make sure programs used by this script exist

call :does_file_exist %TIME%  || exit /b 1
call :does_file_exist %FLUSH% || exit /b 1
call :is_file_installed gawk|| exit /b 1
call :is_file_installed tail|| exit /b 1

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

set file1=tmp.out
set file2=tmp2.out

%FLUSH% 1> Nul 2>&1
%TIME% %smv% -scriptfile %script% %case%  1> %file1% 2>&1
tail -1 tmp.out | gawk "{print $1}" > %file2%
set /p %timevar%=<%file2%
erase %file1% %file2%

exit /b 0

:: -------------------------------------------------------------
  :does_file_exist
:: -------------------------------------------------------------

set file=%1

if NOT exist %file% (
  echo ***fatal error: %file% does not exist
  exit /b 1
)
exit /b 0

:: -------------------------------------------------------------
:is_file_installed
:: -------------------------------------------------------------

  set program=%1
  set file=file_exist.txt
  set filecount=file_exist_count.txt
  %program% --help 1>> %file% 2>&1
  type %file% | find /i /c "not recognized" > %filecount%
  set /p nothave=<%filecount%
  erase %file% %filecount%
  if %nothave% == 1 (
    echo "***Fatal error: %program% not present"
    exit /b 1
  )
  exit /b 0



:eof
cd %CURDIR%


