#!/bin/bash
if [ "`uname`" != "Darwin" ]; then
  PAUSE=1
  echo "shutting down graphics environment (pausing $PAUSE s)"
  sleep $PAUSE
  kill $SMV_ID
fi
