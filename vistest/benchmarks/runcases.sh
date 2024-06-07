#!/bin/bash

QFDS=../../../fds/Utilities/Scripts/qfds.sh

$QFDS -t       plume5c.fds
$QFDS -t       plume5cr.fds
$QFDS -t -p  8 mplume5c8.fds
$QFDS -t -p  8 mplume5c8r.fds
$QFDS -t -p 18 levelset2.fds
$QFDS -t -p 18 levelset2r.fds
$QFDS -t -p  2 thouse5.fds
$QFDS -t -p  2 thouse5r.fds
