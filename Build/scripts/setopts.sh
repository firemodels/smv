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
inc=
BUILD_LIBS=
BUILD_ALL=1
FULL_BUILD=
if [ "$BUILD_ALL" == "1" ]; then
  FULL_BUILD="[default]"
fi
TESTOPT=
while getopts 'AfhiLmprStT' OPTION
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
  echo "-S - build smokeview with sanitize debug options"
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
  r)
  ;;
  S)
   SANITIZE=1
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
if [ "$TESTFLAG" != "" ]; then
   SMV_MAKE_OPTS="$SMV_MAKE_OPTS SMV_TESTFLAG=\"$TESTFLAG\" "
fi

# this parameter is only for the mac
if [ "`uname`" == "Darwin" ]; then
  export GLIBDIROPT
fi
