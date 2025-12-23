#!/bin/bash
if [ "$FIREMODELS == "" ]; then
  export FIREMODELS=$HOME/FireModels_fork
fi
$FIREMODELS/smv\Build/smokeview/intel_win/smokeview_linux $*