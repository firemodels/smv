@echo off
set fdsexe=%userprofile%\FireModels_fork\fds\Build\impi_intel_win_dv\fds_impi_intel_win_dv
%fdsexe% smokex010.fds
%fdsexe% smokex020.fds
%fdsexe% smokex040.fds
%fdsexe% smokex080.fds
%fdsexe% smokex160.fds
%fdsexe% smokex320.fds
smokeview -runscript smokex010
smokeview -runscript smokex020
smokeview -runscript smokex040
smokeview -runscript smokex080
smokeview -runscript smokex160
smokeview -runscript smokex320