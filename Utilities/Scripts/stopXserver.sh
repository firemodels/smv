#!/bin/bash
if [ "`uname`" != "Darwin" ]; then
  display_port=`id -u`
  lockfile=/tmp/startXlock$display_port
  PAUSE=1
  echo "shutting down graphics environment (pausing $PAUSE s)"
  sleep $PAUSE
  kill $SMV_ID
  rm -f $lockfile
fi
