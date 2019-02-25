#!/bin/bash

function usage {
echo "Make_SMV_Pictures.sh [-d -h -r -s size ]"
echo "Generates figures for Smokeview verification suite"
echo ""
echo "Options"
echo "-d - use debug version of smokeview"
echo "-h - display this message"
echo "-i - use installed version of smokeview"
echo "-I - compiler (intel or gnu)"
echo "-t - use test version of smokeview"
echo "-s size - use 32 or 64 bit (default) version of smokeview"
echo "-W - only generate WUI case images"
echo "-Y - generate SMV and WUI case images"
exit
}

is_file_installed()
{
  program=$1
  notfound=`$program -help 2>&1 | tail -1 | grep "not found" | wc -l`
  if [ "$notfound" == "1" ] ; then
    echo "The program $SMV is not available. Run aborted."
    exit
  fi
}

make_helpinfo_files()
{
  dir=$1

  cd $dir
  rm -f *.png
  rm -f *.help

  rm -f smokeview.version
  rm -f smokediff.version
  rm -f smokezip.version
  rm -f background.version
  rm -f wind2fds.version

  rm -f smokeview.help
  rm -f smokediff.help
  rm -f smokezip.help
  rm -f background.help
  rm -f wind2fds.help
  rm -f cfastbot.help
  rm -f firebot.help
  rm -f smokebot.help

  $SMV        -help_all > smokeview.help
  $SMOKEZIP   -help     > smokezip.help
  $SMOKEDIFF  -help     > smokediff.help
  $BACKGROUND -help     > background.help
  $DEM2FDS    -help     > dem2fds.help
  $WIND2FDS   -help     > wind2fds.help
  $CFASTBOT   -H        > cfastbot.help
  $FIREBOT    -H        > firebot.help
  $SMOKEBOT   -H        > smokebot.help

  $SMV -version        > smokeview.version
  $SMOKEZIP -v         > smokezip.version
  $SMOKEDIFF -v        > smokediff.version
  $BACKGROUND -version > background.version
  $DEM2FDS -version    > dem2fds.version
  $WIND2FDS            > wind2fds.version
}

OS=`uname`
if [ "$OS" == "Darwin" ]; then
  PLATFORM=osx
else
  PLATFORM=linux
fi

COMPILER=intel
SIZE=_64
DEBUG=
TEST=
use_installed="0"
RUN_SMV=1
RUN_WUI=1

while getopts 'dghiI:s:tWY' OPTION
do
case $OPTION  in
  d)
   DEBUG=_db
   ;;
  h)
   usage;
   ;;
  i)
   use_installed="1"
   ;;
  I)
   COMPILER="$OPTARG" 
   ;;
  t)
   TEST=_test
  ;;
  s)
   SIZE="$OPTARG"
   if [ $SIZE -eq 64 ] ; then
     SIZE=_64
   else
     SIZE=_32
   fi
  ;;
  W)
   RUN_SMV=0
   RUN_WUI=1
   ;;
  Y)
   RUN_SMV=1
   RUN_WUI=1
   ;;
esac
done
shift $(($OPTIND-1))

VERSION=$PLATFORM$TEST$SIZE$DEBUG
VERSION2=$PLATFORM$SIZE
CURDIR=`pwd`
cd ../../..
export SVNROOT=`pwd`
cd $CURDIR/..

if [ "$use_installed" == "1" ] ; then
  export SMV=smokeview
  export SMOKEZIP=smokediff
  export SMOKEDIFF=smokediff
  export WIND2FDS=wind2fds
  export BACKGROUND=background
  export DEM2FDS=dem2fds
else
  export SMV=$SVNROOT/smv/Build/smokeview/${COMPILER}_$VERSION2/smokeview_$VERSION
  export SMOKEZIP=$SVNROOT/smv/Build/smokezip/${COMPILER}_$VERSION2/smokezip_$VERSION2
  export SMOKEDIFF=$SVNROOT/smv/Build/smokediff/${COMPILER}_$VERSION2/smokediff_$VERSION2
  export WIND2FDS=$SVNROOT/smv/Build/wind2fds/${COMPILER}_$VERSION2/wind2fds_$VERSION2
  export BACKGROUND=$SVNROOT/smv/Build/background/${COMPILER}_$VERSION2/background
  export DEM2FDS=$SVNROOT/smv/Build/dem2fds/${COMPILER}_$VERSION2/dem2fds_$VERSION2
fi
SMOKEBOT=$SVNROOT/bot/Smokebot/run_smokebot.sh
FIREBOT=$SVNROOT/bot/Firebot/run_firebot.sh
CFASTBOT=$SVNROOT/bot/Cfastbot/run_cfastbot.sh

export SMVBINDIR="-bindir $SVNROOT/bot/Bundle/smv/for_bundle"

export STARTX=$SVNROOT/fds/Utilities/Scripts/startXserver.sh
export STOPX=$SVNROOT/fds/Utilities/Scripts/stopXserver.sh

echo Generating smokeview images using:
echo background: $BACKGROUND
echo    dem2fds: $DEM2FDS
echo smokediff : $SMOKEDIFF
echo smokeview : $SMV $SMVBINDIR
echo smokezip  : $SMOKEZIP
echo

RUNSMV=$SVNROOT/fds/Utilities/Scripts/runsmv.sh
export QFDS=$RUNSMV
export RUNCFAST=$RUNSMV
export BASEDIR=`pwd`

export FDSUG=$SVNROOT/fds/Manuals/FDS_User_Guide
export SMVUG=$SVNROOT/smv/Manuals/SMV_User_Guide
export SMVUTILG=$SVNROOT/smv/Manuals/SMV_Utilities_Guide
export SMVVG=$SVNROOT/smv/Manuals/SMV_Verification_Guide
SUMMARY=$SVNROOT/smv/Manuals/SMV_Summary

is_file_installed $SMV
is_file_installed $SMOKEZIP
is_file_installed $SMOKEDIFF
is_file_installed $BACKGROUND
is_file_installed $DEM2FDS
is_file_installed $WIND2FDS

make_helpinfo_files $SMVUG/SCRIPT_FIGURES
make_helpinfo_files $SMVUTILG/SCRIPT_FIGURES

rm -f $SUMMARY/images/*.png

cd $SMVVG/SCRIPT_FIGURES
rm -f *.version
rm -f *.png
$SMV -version > smokeview.version

if [ "$RUN_SMV" == "1" ] ; then
  cd $SVNROOT/smv/Verification/Visualization
  echo Converting particles to isosurfaces in case plumeiso
  $SMOKEZIP -r -part2iso plumeiso

  cd $SVNROOT/smv/Verification/WUI
  echo Converting particles to isosurfaces in case pine_tree
  if  [ -e pine_tree.smv ]; then
    $SMOKEZIP -r -part2iso pine_tree
  fi

# precompute FED slices

  source $STARTX 2>/dev/null
  $QFDS -f -d Visualization plume5c
  $QFDS -f -d Visualization plume5cdelta
  $QFDS -f -d Visualization thouse5
  $QFDS -f -d Visualization thouse5delta
  source $STOPX 2>/dev/null

# difference plume5c and thouse5

  cd $SVNROOT/smv/Verification/Visualization
  echo Differencing cases plume5c and plume5cdelta
  $SMOKEDIFF -w -r plume5c plume5cdelta
  echo Differencing cases thouse5 and thouse5delta
  $SMOKEDIFF -w -r thouse5 thouse5delta

  echo Generating images

  source $STARTX
  cd $SVNROOT/smv/Verification
  scripts/SMV_Cases.sh
  cd $SVNROOT/smv/Verification
  scripts/SMV_DIFF_Cases.sh
  cd $CURDIDR
  source $STOPX

fi

# generate geometry images

if [ "$RUN_WUI" == "1" ] ; then
  source $STARTX
  cd $SVNROOT/smv/Verification
  scripts/WUI_Cases.sh
  source $STOPX
fi

# copy generated images to web summary directory

cp $SMVUG/SCRIPT_FIGURES/*.png $SUMMARY/images/.
cp $SMVVG/SCRIPT_FIGURES/*.png $SUMMARY/images/.

# copy files to utilities script directory for now
cp $SMVUG/SCRIPT_FIGURES/*.png $SMVUTILG/SCRIPT_FIGURES/.
