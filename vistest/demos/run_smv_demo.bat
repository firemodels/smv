@echo off
%set fds=fds
set fds=..\..\..\fds\Build\impi_intel_win\fds_impi_intel_win

call %fds% smv_demo.fds
call %fds% smv_demo2.fds
call %fds% smv_demoa.fds
call %fds% smv_demo2a.fds
