#!/bin/bash
SMV_MAKE_OPTS=
TEST=
TESTFLAG=
SANITIZE=
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
NOQUARTZ=1
inc=
BUILD_LIBS=
BUILD_ALL=1
FULL_BUILD=
if [ "$BUILD_ALL" == "1" ]; then
  FULL_BUILD="[default]"
fi
TESTOPT=
while getopts 'AfhiLmpqQrstT' OPTION
do
case $OPTION in
  A)
   BUILD_ALL=1
  ;;
  f)
   GLUT=freeglut
  ;;
  h)
  echo ""
  echo "options:"
  echo "-a - full build $FULL_BUILD"
  echo "-h - show this help info"
  echo "-i - incremental build"
  echo "-L - rebuild all libraries"
  echo "-p - build a profiling version of smokeview"
  echo "-s - build smokeview with sanitize debug options"
  echo "-t - build a test version of smokeview"
  echo "-T - same as -t"
  exit
  ;;
  i)
   inc=1
   BUILD_ALL=
  ;;
  L)
   BUILD_LIBS=1
  ;;
  p)
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILEFLAG=\"-pg\" "
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILESTRING=\"p\" "
  ;;
  q)
   QUARTZSMV="use_quartz"
   NOQUARTZ=
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"-I /opt/X11/include -Wno-unknown-pragmas"
  ;;
  Q)
   DUMMY=1
  ;;
  s)
   SANITIZE=1
  ;;
  r)
  ;;
  t)
   TESTOPT=1
   ;;
  T)
   TESTOPT=1
  ;;
esac
done

if [ "$TESTOPT" != "" ]; then
   TESTFLAG=" -D pp_BETA"
   SMV_MAKE_OPTS="$SMV_MAKE_OPTS SMV_TESTSTRING=\"test_\" "
   TEST=test_
fi
export SMV_MAKE_OPTS
export GLUT
export TEST
export SANITIZE
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
