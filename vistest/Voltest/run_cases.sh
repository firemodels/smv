#!/bin/bash
FDS=$HOME/FireModels_fork/fds/Build/impi_intel_linux/fds_impi_intel_linux
QFDS="qfds.sh -e $FDS"

$QFDS -p 16 plume16.fds
$QFDS       soot_density.fds
