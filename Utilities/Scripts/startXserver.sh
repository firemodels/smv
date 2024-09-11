#!/bin/bash
IDFILE=$1
DISPLAY_PORT=$2

PAUSE=1

if [ "`uname`" != "Darwin" ]; then
  echo "setting up graphics environment (pausing $PAUSE s)"
  Xvfb :$DISPLAY_PORT -fp /usr/share/X11/fonts/misc -screen 0 1280x1024x24 &
  echo $! > $IDFILE
  export DISPLAY=:$DISPLAY_PORT
  sleep $PAUSE
fi
