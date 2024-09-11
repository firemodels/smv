#!/bin/bash
IDFILE=$1
if [ "`uname`" != "Darwin" ]; then
  PAUSE=1
  echo "shutting down graphics environment (pausing $PAUSE s)"
  sleep $PAUSE
  if [ -e $IDFILE ]; then
    SMV_ID=`head -1 $IDFILE`
    PID=`ps -el | awk '{print $4}' | grep $SMV_ID`
    if [ "$PID" != "" ]; then
      if [ $PID -eq $SMV_ID ]; then
        kill $SMV_ID
        KILLED=1
      fi
    fi
  fi
fi
if [ "$KILLED" != "" ]; then
  echo process $SMV_ID killed >> $IDFILE
else
  if [ "$SMV_ID" != "" ]; then
    echo process $SMV_ID is not running >> $IDFILE
  else
    echo process does not exist >> $IDFILE
  fi
fi
  
