#!/bin/bash
JPREF=$1
if [ "$JPREF" ]; then
 JPREF="-j $JPREF"
fi

$QFDS -p 6 $JPREF -d Visualization/RESTART3 geom3.fds
$QFDS -p 2 $JPREF -d Visualization/RESTART3 test3.fds
