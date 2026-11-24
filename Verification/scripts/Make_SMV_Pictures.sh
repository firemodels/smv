#!/bin/bash

# --------------------- wait_cases_end -----------------------------

wait_cases_end()
{
  while [[ `qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep $JOBPREFIX | grep -v 'C$'` != '' ]]; do
    JOBS_REMAINING=`qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep $JOBPREFIX | grep -v 'C$' | wc -l`
     echo "Waiting for ${JOBS_REMAINING} cases to complete."
     sleep 15
  done
}

# --------------------- usage -----------------------------

function usage {
echo "Make_SMV_Pictures.sh [-d -h -r]"
echo "Generates figures for Smokeview verification suite"
echo ""
echo "Options"
echo "-C - use gnu compiled version of smokeview"
echo "-d - use debug version of smokeview"
echo "-h - display this message"
echo "-i - use installed version of smokeview"
echo "-q q - queue used to generate images"
echo "-t - use test version of smokeview"
exit
}

# --------------------- is_file_installed -----------------------------

is_file_installed()
{
  program=$1
  notfound=`$program -help 2>&1 | tail -1 | grep "not found" | wc -l`
  if [ "$notfound" == "1" ] ; then
    echo "The program $SMV is not available. Run aborted."
    exit
  fi
}

# --------------------- erase_helpinfo_files ----------------------------

erase_helpinfo_files()
{
  dir=$1

  cd $dir
  rm -f *.png
  rm -f *.help

  rm -f smokeview.version
  rm -f wind2fds.version
  rm -f fds.version

  rm -f smokeview.help
  rm -f wind2fds.help
  rm -f cfastbot.help
  rm -f firebot.help
  rm -f smokebot.help
}


# --------------------- make_helpinfo_files ----------------------------

make_helpinfo_files()
{
  dir=$1

  cd $dir

  $SMV        -help_all > smokeview.help
  $WIND2FDS   -help     > wind2fds.help
  $CFASTBOT   -H        > cfastbot.help
  $FIREBOT    -H        > firebot.help
  $SMOKEBOT   -H        > smokebot.help

  $SMV -version        > smokeview.version
  $WIND2FDS            > wind2fds.version
  if [ -e $FDSEXE ]; then
    echo "" | $FDSEXE 2> $GITROOT/smv/Manuals/SMV_User_Guide/SCRIPT_FIGURES/fds.version
  else
    echo "unknown"     > $GITROOT/smv/Manuals/SMV_User_Guide/SCRIPT_FIGURES/fds.version
  fi
}

# ---------------------------------------------------------------------------
# --------------------- beginning of script ---------------------------------
# ---------------------------------------------------------------------------

OS=`uname`
if [ "$OS" == "Darwin" ]; then
  PLATFORM=osx
else
  PLATFORM=linux
fi

if [ ! -d $HOME/.smokebot ]; then
  mkdir $HOME/.smokebot
fi
if [ ! -e $HOME/.smokebot/xvfb_ids ]; then
  echo 1000 > $HOME/.smokebot/xvfb_ids
fi

COMPILER=intel
DEBUG=
TEST=
use_installed=
QUEUE=batch
CPUS_PER_TASK=

while getopts 'Cdhij:q:tT:' OPTION
do
case $OPTION  in
  C)
   COMPILER=gnu
   ;;
  d)
   DEBUG=_db
   ;;
  h)
   usage;
   ;;
  i)
   use_installed="-i"
   ;;
  j)
   JOBPREFIX="$OPTARG" 
   ;;
  q)
   QUEUE="$OPTARG" 
   ;;
  t)
   TEST=_test
  ;;
  T)
   CPUS_PER_TASK="-T $OPTARG" 
   ;;
esac
done
shift $(($OPTIND-1))

if [ "$JOBPREFIX" == "" ]; then
  JOBPREFIX=SMV_
fi

VERSION=$PLATFORM${TEST}$DEBUG
VERSION2=${PLATFORM}
CURDIR=`pwd`
cd ../../..
export GITROOT=`pwd`
cd $CURDIR/..
export BASEDIR=`pwd`

PREFIX=i
FDSEXE=$GITROOT/fds/Build/${PREFIX}mpi_${COMPILER}_$PLATFORM/fds_${PREFIX}mpi_${COMPILER}_$PLATFORM
if [ ! -e $FDSEXE ]; then
  PREFIX=o  
  FDSEXE=$GITROOT/fds/Build/${PREFIX}mpi_${COMPILER}_$PLATFORM/fds_${PREFIX}mpi_${COMPILER}_$PLATFORM
fi

if [ "$use_installed" == "1" ] ; then
  export SMV=smokeview
  export FDS2FED=fds2fed
  export PART2ISO=part2iso
  export WIND2FDS=wind2fds
  export SMVBINDIR=`which smokeview`
  if [ "$SMVBINDIR" != "" ]; then
    SMVBINDIR=${SMVBINDIR%/*}
  fi
else
  export SMV=$GITROOT/smv/Build/smokeview/${COMPILER}_$VERSION2/smokeview_$VERSION
  export FDS2FED=$GITROOT/smv/Build/fds2fed/${COMPILER}_$VERSION2/fds2fed_$VERSION2
  export PART2ISO=$GITROOT/smv/Build/part2iso/${COMPILER}_$VERSION2/part2iso_$VERSION2
  export WIND2FDS=$GITROOT/smv/Build/wind2fds/${COMPILER}_$VERSION2/wind2fds_$VERSION2
  export SMVBINDIR=$GITROOT/smv/Build/for_bundle
fi

SMOKEBOT=$GITROOT/bot/Smokebot/run_smokebot.sh
FIREBOT=$GITROOT/bot/Firebot/run_firebot.sh
CFASTBOT=$GITROOT/bot/Cfastbot/run_cfastbot.sh

echo Generating smokeview images using:
echo smokeview : $SMV
echo

RUNSMV="$GITROOT/smv/Utilities/Scripts/qsmv.sh $CPUS_PER_TASK -j $JOBPREFIX $use_installed -q $QUEUE"
export QFDS=$RUNSMV
export RUNCFAST=$RUNSMV

export FDSUG=$GITROOT/fds/Manuals/FDS_User_Guide
export SMVUG=$GITROOT/smv/Manuals/SMV_User_Guide
export SMVVG=$GITROOT/smv/Manuals/SMV_Verification_Guide
SUMMARY=$GITROOT/smv/Manuals/SMV_Summary

is_file_installed $SMV
is_file_installed $PART2ISO
is_file_installed $WIND2FDS

erase_helpinfo_files $SMVUG/SCRIPT_FIGURES

rm -f $SUMMARY/images/*.png

cd $SMVVG/SCRIPT_FIGURES
rm -f *.version
rm -f *.png

make_helpinfo_files $SMVUG/SCRIPT_FIGURES

$SMV -version > smokeview.version

# precompute FED slices
cd $GITROOT/smv/Verification/Visualization
$FDS2FED plume5c            &
pid_feda=$1
$FDS2FED plume5cdelta       &
pid_fedb=$1
$FDS2FED thouse5            &
pid_fedc=$1
$FDS2FED thouse5delta       &
pid_fedd=$1
$FDS2FED fed_test           &
pid_fede=$1

# compute isosurface from particles

cd $GITROOT/smv/Verification/Visualization
echo Converting particles to isosurfaces in case plumeiso
$PART2ISO plumeiso &
pid_caseb=$!

cd $GITROOT/smv/Verification/WUI
if  [ -e pine_tree.smv ]; then
  echo Converting particles to isosurfaces in case pine_tree
  $PART2ISO pine_tree &
  pid_casec=$!
fi

wait $pid_feda
wait $pid_fedb
wait $pid_fedc
wait $pid_fedd
wait $pid_fede
wait $pid_caseb
if  [ -e $GITROOT/smv/Verification/WUI/pine_tree.smv ]; then
  wait $pid_casec
fi

echo Generating images

cd $GITROOT/smv/Verification
scripts/SMV_Cases.sh
scripts/RESTART_Cases.sh
cd $GITROOT/smv/Verification
cd $CURDIDR

# generate geometry images

cd $GITROOT/smv/Verification
scripts/WUI_Cases.sh

wait_cases_end

# copy generated images to web summary directory

cp $SMVUG/SCRIPT_FIGURES/*.png $SUMMARY/images/.
cp $SMVVG/SCRIPT_FIGURES/*.png $SUMMARY/images/.
