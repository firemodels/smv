#!/bin/bash

# --------------------- wait_cases_end -----------------------------

wait_cases_end()
{
  while [[ `qstat -a | awk '{print $2 $4}' | grep $(whoami) | grep ${MOV_JOBPREFIX}` != '' ]]; do
     JOBS_REMAINING=`qstat -a | awk '{print $2 $4}' | grep $(whoami) | grep ${MOV_JOBPREFIX} | wc -l`
     echo "Waiting for ${JOBS_REMAINING} cases to complete."
     sleep 15
  done
}

MOV_JOBPREFIX=MOV_

TEST=$1

size=_64

OS=`uname`
if [ "$OS" == "Darwin" ]; then
  PLATFORM=_osx
else
  PLATFORM=_linux
fi

CURDIR=`pwd`
cd ../..
GITROOT=`pwd`
cd $CURDIR

FDSEXE=$GITROOT/fds/Build/mpi_intel$PLATFORM$size/fds_mpi_intel$PLATFORM$size
MAKEMOVIE="$GITROOT/fds/Utilities/Scripts/make_movie.sh"
QFDS=$GITROOT/fds/Utilities/Scripts/qfds.sh
QSMV="$GITROOT/smv/Utilities/Scripts/qsmv.sh -j ${MOV_JOBPREFIX}"

VDIR=$GITROOT/smv/Verification
INDIR=$GITROOT/smv/Verification/Visualization/frames
WUIINDIR=$GITROOT/smv/Verification/WUI/frames
OUTDIR=$GITROOT/smv/Manuals/SMV_Summary/movies

rm -f $INDIR/*.png
rm -f $WUIINDIR/*.png
rm -f $OUTDIR/*.m1v
rm -f $OUTDIR/*.mp4
rm -f $OUTDIR/*.png

# create version strings

cd $VDIR
$QFDS -e $FDSEXE -d Visualization -q terminal version2.fds
$QSMV            -d Visualization             version2

# -------- make movie frames -------------------

cd $VDIR

$QSMV -p 8 -c plume5c_movie        -d Visualization plume5c
$QSMV -p 8 -c thouse_movie         -d Visualization thouse5
$QSMV -p 8 -c BT10m_2x2km_LS_movie -d WUI           BT10m_2x2km_LS
$QSMV -p 8 -c levelset1            -d WUI           levelset1
$QSMV -p 8 -c wind_test1           -d WUI           wind_test1
$QSMV -p 8 -c tree_test2           -d WUI           tree_test2
wait_cases_end

# -------- make movies -------------------

$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m plume5c_tslice  plume5c_tslice"
$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m plume5c_3dsmoke plume5c_3dsmoke"
$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m plume5c_vtslice plume5c_vtslice"
$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m plume5c_iso     plume5c_iso"
$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m plume5c_tbound  plume5c_tbound"
$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m plume5c_part    plume5c_part"
$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m thouse5_tslice  thouse5_tslice"
$QSMV -d $INDIR    -C "$MAKEMOVIE -o $OUTDIR -m thouse5_smoke3d thouse5_smoke3d"

$QSMV -d $WUIINDIR -C "$MAKEMOVIE -o $OUTDIR BT10m_2x2km_LS"
$QSMV -d $WUIINDIR -C "$MAKEMOVIE -o $OUTDIR hill_structure"
$QSMV -d $WUIINDIR -C "$MAKEMOVIE -o $OUTDIR levelset1"
$QSMV -d $WUIINDIR -C "$MAKEMOVIE -o $OUTDIR wind_test1"
$QSMV -d $WUIINDIR -C "$MAKEMOVIE -o $OUTDIR tree_test2"
wait_cases_end

echo movies generated

