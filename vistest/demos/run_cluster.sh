#!/bin/bash
QFDS="qfds.sh -I -p 8 -e ../../../fds/Build/impi_intel_linux_64/fds_impi_intel_linux_64"

$QFDS chassis.fds
$QFDS cluster1.fds
