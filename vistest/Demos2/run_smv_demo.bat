@echo off
%set fds=fds
set fds=..\..\..\fds\Build\impi_intel_win_64\fds_impi_intel_win_64

call %fds% smv_demo.fds
call %fds% smv_demo2.fds
call %fds% smv_demoa.fds
call %fds% smv_demo2a.fds
