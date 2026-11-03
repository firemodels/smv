#!/bin/bash
if [ "$FIREMODELS" == "" ]; then
  export FIREMODELS=$HOME/FireModels_fork
fi
"$FIREMODELS/smv/Build/smokeview/intel_linux/smokeview_linux_test" "$@"
