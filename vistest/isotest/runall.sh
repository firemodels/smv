#!/bin/bash

QUEUE="-q batch4"

qfds.sh -p 16 $QUEUE plume_1p5cm.fds
qfds.sh -p 16 $QUEUE plume_3cm.fds
qfds.sh -p 16 $QUEUE plume_6cm.fds

