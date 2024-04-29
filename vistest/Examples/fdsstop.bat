@echo off
set chid=%1
echo 1 > %chid%.stop
if exist %chid%.running erase %chid%.running
