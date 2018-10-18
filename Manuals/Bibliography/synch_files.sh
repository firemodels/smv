#!/bin/bash
fdsdir=../../../fds/Manuals/Bibliography
cp $fdsdir/FDS_general.bib .
cp $fdsdir/FDS_mathcomp.bib .
cp $fdsdir/FDS_refs.bib .
cp $fdsdir/authors.tex .
fdsdir=../../../fds/Source
cp $fdsdir/gsmv.f90 ../../Source/smokeview/.
