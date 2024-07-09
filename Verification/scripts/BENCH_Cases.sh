#!/bin/bash
$QFDS -d Visualization      -t mplume5c8_bench.fds
$QFDS -d Visualization      -t plume5c_bench.fds
$QFDS -d Visualization -p 2 -t thouse5_bench.fds

$QFDS -d WUI -t -A wind_test2_bench.fds
