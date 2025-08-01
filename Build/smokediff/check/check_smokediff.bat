@echo off

make -j 8 SHELL="%ComSpec%" -f ../Makefile win_check
