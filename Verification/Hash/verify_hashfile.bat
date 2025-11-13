@echo off

set CURDIR=%CD%
set HASHFILEDIR=..\..\Build\hashfile\intel_win
cd %HASHFILEDIR%
set HASHFILEDIR=%CD%
set HASHFILE=%HASHFILEDIR%\hashfile_win.exe
cd %CURDIR%

if not exist %HASHFILE% (
  echo "***error: the program %HASHFILE% does not exist"
  echo "verification aborted"
)

%HASHFILE% -hash_all base > result
diff result reference_win | wc -l > diffcount.out
set /p diffcount=<diffcount.out
if %diffcount% EQU 0 (
  echo PASS! The hashes computed by %HASHFILE% match the reference file.
) else (
  echo FAIL! The hashes computed by %HASHFILE% do not match the reference file.
)
erase diffcount.out
