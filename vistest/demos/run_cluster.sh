#!/bin/bash
QFDS="qfds.sh -I -p 8 -e ../../../fds/Build/impi_intel_linux_64/fds_impi_intel_linux_64"

$QFDS cluster1.fds
$QFDS cluster1b.fds
$QFDS cluster2.fds
$QFDS cluster2b.fds
