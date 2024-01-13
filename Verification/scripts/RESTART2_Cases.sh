#!/bin/bash

$QFDS -p 6 -j RES -d Visualization/RESTART3 geom3.fds
$QFDS -p 2 -j RES -d Visualization/RESTART3 test3.fds
