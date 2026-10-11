#!/bin/bash
REPOROOT=$1

# -------------------------------------------------------------

BUILDSMVLIBS()
{
  cd $smvrepo/Build/LIBS/intel_linux
  ./make_LIBS.sh bot >> $outputdir/compile_smvlibs.log 2>&1 
}

# -------------------------------------------------------------

BUILD()
{
  prog=$1

  cd $smvrepo/Build/$prog/intel_linux
  ./make_${prog}.sh bot                    >> $outputdir/compile_$prog.log 2>&1
}

# -------------------------------------------------------------

CHECK_BUILD()
{
  prog=$1

  if [ ! -e $smvrepo/Build/$prog/intel_linux/${prog}_linux ]; then
    echo "***error: The program ${prog}_linux failed to build"
    echo "***error: The program ${prog}_linux failed to build"   >> $errorlog 2>&1
  else
    echo ${prog}_linux built
  fi
}

#--------------------- start of script -------------------------------

CURDIR=`pwd`

outputdir=$CURDIR/output
errorlog=$CURDIR/output/smverror.log

echo > $errorlog

cd $REPOROOT/smv
smvrepo=`pwd`

cd $REPOROOT/bot
botrepo=`pwd`

# build smokeview libraries
echo building smokeview libraries
BUILDSMVLIBS &
pid_smvlibs=$!

echo building fds2fed
BUILD fds2fed &
pid_fds2fed=$!

echo building smokediff
BUILD smokediff &
pid_smokediff=$!

echo building wind2fds
BUILD wind2fds &
pid_wind2fds=$!

#wait for smokeview libraries to be built before continuing
wait $pid_smvlibs

echo building pnginfo
BUILD pnginfo &
pid_pnginfo=$!

echo building smokeview
BUILD smokeview &
pid_smokeview=$!

echo building smokezip
BUILD smokezip &
pid_smokezip=$!

# verify smokeview apps were built

wait $pid_fds2fed
CHECK_BUILD fds2fed

wait $pid_pnginfo
CHECK_BUILD pnginfo

wait $pid_smokediff
CHECK_BUILD smokediff

wait $pid_smokeview
CHECK_BUILD smokeview

wait $pid_smokezip
CHECK_BUILD smokezip

wait $pid_wind2fds
CHECK_BUILD wind2fds

echo smv app builds complete

cd $CURDIR
 
