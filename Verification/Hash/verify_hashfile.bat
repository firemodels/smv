@echo off

set HASHFILE=..\..\Build\hashfile\intel_win_64\hashfile_win_64.exe
if not exist %HASHFILE% (
  echo "***error: the program %HASHFILE% does not exist"
  echo "verification aborted"
)

%HASHFILE% -hash_all base > result
diff result reference_win | wc -l > diffcount.out
set /p diffcount=<diffcount.out
if %diffcount% EQU 0 (
  echo PASS! The hashes computed by hashfile match the reference file.
) else (
  echo FAIL! The hashes computed by hashfile do not match the reference file.
)
erase diffcount.out
