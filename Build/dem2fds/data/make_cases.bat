 @echo off
set option=%1
set option2=%2

set dem2fds=..\intel_win_64\dem2fds_win_64.exe
if NOT exist %dem2fds% set dem2fds=dem2fds
::set dem2fds=dem2fds

set "GOOGLE=%userprofile%\Google Drive\terrain"
set HOME=%userprofile%\terrain

set "DIR=%GOOGLE%"
if exist %HOME% set "DIR=%HOME%"

%dem2fds% %option% %option2% -dir "%DIR%\blodget" blodget.in 

%dem2fds% %option% %option2% -elevdir "%DIR%\N40W078" -dir "%DIR%\demtest" demtest1.in 
%dem2fds% %option% %option2% -elevdir "%DIR%\N40W078" -dir "%DIR%\demtest" demtest2.in 

%dem2fds% %option% %option2% -elevdir "%DIR%\N40W078" -dir "%DIR%\nist" nist.in 

%dem2fds% %option% %option2% -elevdir "%DIR%\N40W078" -dir "%DIR%\sugarloaf" sugarloaf.in 

%dem2fds% %option% %option2% -overlap -dir "%DIR%\tower" tower.in 

%dem2fds% %option% %option2% -dir "%DIR%\trails" trails.in 
%dem2fds% %option% %option2% -dir "%DIR%\trails" trails2.in 
