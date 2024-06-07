#!/bin/bash

QFDS=../../../fds/Utilities/Scripts/qfds.sh

$QFDS -t -p  2 thouse5.fds
$QFDS -t -p  2 thouse5r.fds
$QFDS -t -p  8 mplume5c8.fds
$QFDS -t -p  8 mplume5c8r.fds
$QFDS -t -p 18 levelset2.fds
$QFDS -t -p 18 levelset2r.fds
$QFDS -t -p 64 plume64.fds
$QFDS -t -p 64 plume64r.fds
