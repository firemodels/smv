#!/bin/bash
QFDS="../../../fds/Utilities/Scripts/qfds.sh -I "

$QFDS -p   4      chassis1.fds
$QFDS -p  32 -n 8 chassis2.fds
$QFDS -p  4       chassis3.fds
$QFDS -p  16 -n 8 cluster1.fds
$QFDS -p 128 -n 8 cluster2.fds
