#!/bin/bash
IDFILE=$1
DISPLAY_PORT=$2

display_port=`id -u`
PAUSE=1
GETNEWPORT () 
{
  nmatches=`ps a -e | grep Xvfb | grep $display_port | grep -v grep | wc | awk '{print $1}'`
  while [ $nmatches -ne 0 ] ; do
    display_port=`expr $display_port + 1`
    nmatches=`ps a -e | grep Xvfb | grep $display_port | grep -v grep | wc | awk '{print $1}'`
  done
}

if [ "`uname`" != "Darwin" ]; then
  echo "setting up graphics environment (pausing $PAUSE s)"
  if [ "$DISPLAYPPORT" == "" ]; then
    GETNEWPORT 
  else
    display_port=$DISPLAY_PORT
  fi
  Xvfb :$display_port -fp /usr/share/X11/fonts/misc -screen 0 1280x1024x24 &
  echo $! > $IDFILE
  export DISPLAY=:$display_port
  sleep $PAUSE
fi
