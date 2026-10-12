@echo off
set pull_number=%1
git fetch firemodels pull/%pull_number%/head:test_%pull_number%