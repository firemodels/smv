@echo off

set HASHFILE=..\..\Build\hashfile\intel_win_64\hashfile_win_64.exe
if not exist %HASHFILE% (
  echo "***error: the program %HASHFILE% does not exist"
  echo "verification aborted"
)

%HASHFILE% -hash_all base > result
fc result reference_win 
