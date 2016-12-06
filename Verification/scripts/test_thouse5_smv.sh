#!/bin/bash

function usage {
echo "Make_SMV_Pictures.sh [-d -h -r -s size ]"
echo "Generates figures for Smokeview verification suite"
echo ""
echo "Options"
echo "-d - use debug version of smokeview"
echo "-g - only generate geometry case images"
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
RUN_GEOM=0
RUN_WUI=1

while getopts 'dghiI:s:tWY' OPTION
do
case $OPTION  in
  d)
   DEBUG=_db
   ;;
  g)
   RUN_SMV=0
   RUN_GEOM=1
   RUN_WUI=0
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
   RUN_GEOM=0
   RUN_WUI=1
   ;;
  Y)
   RUN_SMV=1
   RUN_GEOM=0
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
else
  export SMV=$SVNROOT/smv/Build/smokeview/${COMPILER}_$VERSION2/smokeview_$VERSION
  export SMOKEZIP=$SVNROOT/smv/Build/smokezip/${COMPILER}_$VERSION2/smokezip_$VERSION2
  export SMOKEDIFF=$SVNROOT/smv/Build/smokediff/${COMPILER}_$VERSION2/smokediff_$VERSION2
  export WIND2FDS=$SVNROOT/smv/Build/wind2fds/${COMPILER}_$VERSION2/wind2fds_$VERSION2
  export BACKGROUND=$SVNROOT/smv/Build/background/${COMPILER}_$VERSION2/background
fi

export SMVBINDIR="-bindir $SVNROOT/smv/for_bundle"

export STARTX=$SVNROOT/fds/Utilities/Scripts/startXserver.sh
export STOPX=$SVNROOT/fds/Utilities/Scripts/stopXserver.sh

RUNSMV=$SVNROOT/fds/Utilities/Scripts/runsmv.sh
export QFDS=$RUNSMV
export RUNCFAST=$RUNSMV
export BASEDIR=`pwd`

export FDSUG=$SVNROOT/fds/Manuals/FDS_User_Guide
export SMVUG=$SVNROOT/smv/Manuals/SMV_User_Guide
export SMVVG=$SVNROOT/smv/Manuals/SMV_Verification_Guide
SUMMARY=$SVNROOT/smv/Manuals/SMV_Summary

cd $SMVUG/SCRIPT_FIGURES
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

rm -f $SUMMARY/images/*.png
source ~/.bashrc_fds

  source $STARTX
  cd $SVNROOT/smv/Verification
for host in blaze001 blaze002 blaze003 blaze004 blaze005 blaze006 blaze007 blaze008 blaze009 blaze010 blaze011 blaze012 blaze013 blaze014 blaze015 blaze016 blaze017 blaze019 blaze020 blaze021 blaze022 blaze023 blaze024 blaze025 blaze026 blaze027 blaze028 blaze029 blaze030 blaze031 blaze032 
do
echo "-------------------------------------------------"
echo $host
echo "-------------------------------------------------"
$QFDS q smokebot -d $host thouse5.fds
done
source $STOPX

