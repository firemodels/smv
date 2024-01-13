#!/bin/bash

$QFDS -p 6 -j RES -d Visualization/RESTART1 geom1.fds
$QFDS -p 2 -j RES -d Visualization/RESTART1 test1.fds
$QFDS -p 6 -j RES -d Visualization/RESTART2 geom2.fds
$QFDS -p 2 -j RES -d Visualization/RESTART2 test2.fds
