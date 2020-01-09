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
QUARTZ=framework
inc=
while getopts 'fhimpqrt' OPTION
do
case $OPTION in
  f)
   GLUT=freeglut
  ;;
  h)
  echo "options:"
  echo "-p - build a profiling version of smokeview"
  echo "-t - build a test version of smokeview"
  exit
  ;;
  i)
   inc=1
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
   QUARTZ=use_quartz
  ;;
  r)
  ;;
  t)
   TESTFLAG=$TESTFLAG" -D pp_BETA"
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_TESTSTRING=\"test_\" "
   TEST=test_
  ;;
esac
done
export SMV_MAKE_OPTS
export GLUT
export TEST
export SMV_MPI
if [ "$TESTFLAG" != "" ]; then
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_TESTFLAG=\"$TESTFLAG\" "
fi
# this parameter is only for the mac
if [ "`uname`" == "Darwin" ]; then
  export QUARTZ
  export GLIBDIROPT
else
  QUARTZ=
fi
