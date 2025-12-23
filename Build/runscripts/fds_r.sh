#!/bin/bash
if [ "$FIREMODELS" == "" ]; then
  FIREMODELS=$HOME/FireModels_fork
fi
$FIREMODELS/fds/Build/impi_intel_linux/fds_impi_intel_linux $*
