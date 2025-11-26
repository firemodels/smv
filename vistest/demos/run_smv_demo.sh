#!/bin/bash
QFDS="qfds.sh -I -e ../../../fds/Build/impi_intel_linux/fds_impi_intel_linux"

$QFDS smv_demo.fds
$QFDS -p 4 smv_demo2.fds
$QFDS smv_demoa.fds
$QFDS -p 4 smv_demo2a.fds
