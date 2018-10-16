@echo off
set fdsdir=..\..\..\fds\Manuals\Bibliography
copy %fdsdir%\FDS_general.bib .
copy %fdsdir%\FDS_mathcomp.bib .
copy %fdsdir%\FDS_refs.bib .
copy %fdsdir%\authors.tex .
set fdsdir=..\..\..\fds\Source
copy %fdsdir%\gsmv.f90 ..\..\Source\smokeview\gsmv.f90
