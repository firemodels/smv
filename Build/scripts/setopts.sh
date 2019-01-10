#!/bin/bash
SMV_MAKE_OPTS=
TEST=
GLUT=glut
QUARTZ=framework
while getopts 'fhpqrt' OPTION
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
  p)
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILEFLAG=\"-p\" "
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILESTRING=\"profile\" "
  ;;
  q)
   QUARTZ=use_quartz
  ;;
  r)
  ;;
  t)
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_TESTFLAG=\"-D pp_BETA\" "
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_TESTSTRING=\"test_\" "
   TEST=test_
  ;;
esac
done
export SMV_MAKE_OPTS
export GLUT
export TEST
# this parameter is only for the mac
if [ "`uname`" == "Darwin" ]; then
export QUARTZ
else
QUARTZ=
fi
