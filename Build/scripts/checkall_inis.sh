#!/bin/bash
GLOB=$*
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
CHECK_INI=$SCRIPT_DIR/check_ini.sh
for file in $GLOB; do
  $CHECK_INI $file
done
