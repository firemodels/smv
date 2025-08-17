#!/bin/bash
$QFDS -p 2  -d WUI hill_structure.fds
$QFDS -p 9  -d WUI levelset2.fds
$QFDS -p 16 -d WUI pine_tree.fds
$QFDS       -d WUI tree_test2.fds
$QFDS       -d WUI wind_test1.fds
$QFDS -p 2  -d WUI wind_test2.fds
