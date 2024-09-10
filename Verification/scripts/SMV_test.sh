#!/bin/bash
QUEUE=$1
$QFDS -q $QUEUE -d Visualization plume5c.fds
$QFDS -q $QUEUE -d Visualization thouse5.fds
