#!/bin/bash
bibdir=../../../fds/Manuals/Bibliography
scriptdir=../../../fds/Manuals/scripts
cp $scriptdir/check_manuals.py ../scripts/.

cd ../Bibliography
cp $bibdir/FDS_general.bib .
cp $bibdir/FDS_mathcomp.bib .
cp $bibdir/FDS_refs.bib .
cp $bibdir/authors.tex .
