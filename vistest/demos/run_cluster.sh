#!/bin/bash
QFDS="qfds.sh -p 4 -e ../../../fds/Build/mpi_intel_linux_64ib/fds_mpi_intel_linux_64ib"

$QFDS cluster1.fds
$QFDS cluster1b.fds
$QFDS cluster2.fds
$QFDS cluster2b.fds
