#!/bin/bash

QUEUE="-q batch4"

qfds.sh -I -p 16 $QUEUE plume_1p5cm.fds
qfds.sh -I -p 16 $QUEUE plume_3cm.fds
qfds.sh -I -p 16 $QUEUE plume_6cm.fds
qfds.sh -I $QUEUE plume_6cmb.fds
