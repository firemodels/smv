#!/bin/bash
QFDS="../../../fds/Utilities/Scripts/qfds.sh -I "

#$QFDS -p 4  chassis1.fds
#$QFDS -p 32 chassis2.fds
#$QFDS -p 16 cluster1.fds
$QFDS -p 128 -n 8 cluster2.fds
