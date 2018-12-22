#!/bin/bash
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

export SMV=$GITROOT/smv/Build/smokeview/intel$PLATFORM$size/smokeview$PLATFORM$TEST$size
FDSEXE=$GITROOT/fds/Build/mpi_intel$PLATFORM$size/fds_mpi_intel$PLATFORM$size
RUNSMV="$GITROOT/fds/Utilities/Scripts/runsmv.sh"
export SMVBINDIR="-bindir $GITROOT/bot/Bundle/smv/for_bundle"
MAKEMOVIE="$GITROOT/fds/Utilities/Scripts/make_movie.sh"
STARTX=$GITROOT/fds/Utilities/Scripts/startXserver.sh
STOPX=$GITROOT/fds/Utilities/Scripts/stopXserver.sh
QFDS=$GITROOT/fds/Utilities/Scripts/qfds.sh

export BASEDIR=`pwd`

if ! [ -e $SMV ]; then
  echo "*** Error: The program $SMV does not exist. Run aborted."
  exit
fi

underscore=_
VDIR=$GITROOT/smv/Verification
INDIR=$GITROOT/smv/Verification/Visualization/frames
WUIINDIR=$GITROOT/smv/Verification/WUI/frames
OUTDIR=$GITROOT/smv/Manuals/SMV_Summary/movies

rm -f $INDIR/*.png
rm -f $WUIINDIR/*.png
rm -f $OUTDIR/*.m1v
rm -f $OUTDIR/*.mp4
rm -f $OUTDIR/*.png

# make a movie
MKMOVIE()
{
  CASEDIR=$1
  BASEFILE=$2
  FRAMEDIR=$3

  cd $VDIR

# generate movie frames
  $RUNSMV -m -d $CASEDIR $BASEFILE

  cd $FRAMEDIR

# make movies out of frames generated above
  echo making $BASEFILE movie
  $MAKEMOVIE -o $OUTDIR ${BASEFILE}_movie  > /dev/null
}

# start background X server for picture generation
echo Starting background X server
source $STARTX

# create version string

cd $VDIR
$QFDS -e $FDSEXE -d Visualization -q terminal version2.fds

cd $VDIR
$RUNSMV -d Visualization version2

# The -m option assumes that a script
# named casename_movies.ssf exists for each 

# -------- plume5c movie -------------------

cd $VDIR

# generate movie frames
$RUNSMV -m -d Visualization plume5c

cd $INDIR

# make movies out of frames generated above
echo making plume5c_tslice movie
$MAKEMOVIE -o $OUTDIR -m plume5c_tslice plume5c_tslice > /dev/null

echo making plume5c_3dsmoke movie
$MAKEMOVIE -o $OUTDIR  -m plume5c_3dsmoke plume5c_3dsmoke  > /dev/null

echo making plume5c_vtslice movie
$MAKEMOVIE -o $OUTDIR -m plume5c_vtslice plume5c_vtslice > /dev/null

echo making plume5c_iso movie
$MAKEMOVIE -o $OUTDIR  -m plume5c_iso plume5c_iso  > /dev/null

echo making plume5c_tbound movie
$MAKEMOVIE -o $OUTDIR  -m plume5c_tbound plume5c_tbound > /dev/null

echo making plume5c_part movie
$MAKEMOVIE -o $OUTDIR  -m plume5c_part plume5c_part  > /dev/null

# -------- thouse5 movies -------------------

cd $VDIR

# generate movie frames
$RUNSMV -m -d Visualization thouse5

cd $INDIR

# make movies out of frames generated above
echo making thouse5_tslice movie
$MAKEMOVIE -o $OUTDIR  -m thouse5_tslice thouse5_tslice  > /dev/null

echo making thouse5_smoke3d movie
$MAKEMOVIE -o $OUTDIR  -m thouse5_smoke3d thouse5_smoke3d > /dev/null

MKMOVIE WUI BT10m_2x2km_LS $WUIINDIR
MKMOVIE WUI hill_structure $WUIINDIR
MKMOVIE WUI levelset1 $WUIINDIR
MKMOVIE WUI wind_test1 $WUIINDIR
MKMOVIE WUI tree_test2 $WUIINDIR

source $STOPX
