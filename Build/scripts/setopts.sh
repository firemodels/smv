#!/bin/bash
SMV_MAKE_OPTS=
TEST=
while getopts 'hprt' OPTION
do
case $OPTION in
  h)
  echo "options:"
  echo "-p - build a profiling version of smokeview"
  echo "-t - build a test version of smokeview"
  exit
  ;;
  p)
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILEFLAG=\"-p\" "
   SMV_MAKE_OPTS=$SMV_MAKE_OPTS"SMV_PROFILESTRING=\"p\" "
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
if [ "$IFORT_COMPILER" == "" ] ; then
  echo warning IFORT_COMPILER environment variable is not defined
  echo defining as /opt/composerxe
  export IFORT_COMPILER=/opt/intel/composerxe
fi
if [ "$IFORT_COMPILER_LIB" == "" ] ; then
  echo warning IFORT_COMPILER_LIB is not defined
  echo defining as $IFORT_COMPILER/lib
  export IFORT_COMPILER_LIB=$IFORT_COMPILER/lib
fi
