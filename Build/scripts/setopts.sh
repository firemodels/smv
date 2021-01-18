#!/bin/bash
SMV_MAKE_OPTS=
TEST=
SMV_MPI=
TESTFLAG=
if [ "`uname`" == "Darwin" ]; then
  GLUT=
  GLIBDIROPT=
  if [ "$GLIBDIR" != "" ]; then
    GLIBDIROPT=-L$GLIBDIR
  fi
else
  GLUT=glut
fi
QUARTZSMV=framework
inc=
BUILD_LIBS=
BUILD_ALL=1
while getopts 'AfhiLmpqQrT' OPTION
do
case $OPTION in
  A)
   BUILD_ALL=1
  ;;
  f)
   GLUT=freeglut
  ;;
  h)
  echo "options:"
  echo "-a - full build"
  echo "-i - an incremental build"
  echo "-L - build all libraries"
  echo "-p - build a profiling version of smokeview"
  echo "-t - build a test version of smokeview"
  exit
  ;;
  i)
   inc=1
   BUILD_ALL=
  ;;
  L)
   BUILD_LIBS=1
  ;;
  m)
   SMV_MPI=1
   TESTFLAG=$TESTFLAG" -D pp_MPI"
  ;;
  p)
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILEFLAG=\"-pg\" "
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILESTRING=\"p\" "
  ;;
  q)
   QUARTZSMV="use_quartz"
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"-I /opt/X11/include -Wno-unknown-pragmas"
  ;;
  Q)
   DUMMY=1
  ;;
  r)
  ;;
  T)
   TESTFLAG=$TESTFLAG" -D pp_BETA"
   SMV_MAKE_OPTS="$SMV_MAKE_OPTS SMV_TESTSTRING=\"test_\" "
   TEST=test_
  ;;
esac
done
export SMV_MAKE_OPTS
export GLUT
export TEST
export SMV_MPI
if [ "$NOQUARTZ" != "" ]; then
  TESTFLAG="$TESTFLAG -D pp_NOQUARTZ"
  SMV_MAKE_OPTS="$SMV_MAKE_OPTS NOQUARTZ=\"\" "
else
  SMV_MAKE_OPTS="$SMV_MAKE_OPTS NOQUARTZ=\"q_\" "
fi
if [ "$TESTFLAG" != "" ]; then
   SMV_MAKE_OPTS="$SMV_MAKE_OPTS SMV_TESTFLAG=\"$TESTFLAG\" "
fi

# this parameter is only for the mac
if [ "`uname`" == "Darwin" ]; then
  export QUARTZSMV
  export GLIBDIROPT
else
  QUARTZSMV=
fi
