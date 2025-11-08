#!/bin/bash

notfound=`xvfb-run 2>&1 >/dev/null | tail -1 | grep "not found" | wc -l`
if [ $notfound -eq 1 ]; then
   echo "***error: xvfb-run not installed"
else
   xvfb-run -s "-fp /usr/share/X11/fonts/misc -screen 0 1280x1024x24" -a $*
fi
