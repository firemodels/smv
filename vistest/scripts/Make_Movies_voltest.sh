#!/bin/bash

makemovie=/usr/local/bin/make_movie.sh

CURDIR=`pwd`
export GITROOT=`pwd`/../../..
cd $GITROOT
export GITROOT=`pwd`

OUTDIR=$GITROOT/Manuals/SMV_Summary/movies2

cd $CURDIR/..

#$makemovie -i Voltest/frames -o $OUTDIR mplume8n
#$makemovie -i Voltest/frames -o $OUTDIR mplumeB8n
$makemovie -i Voltest/frames -o $OUTDIR voltest2
