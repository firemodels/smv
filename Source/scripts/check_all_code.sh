#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
for file in *.c *.cpp; do
   echo analysing $file
   $SCRIPT_DIR/check_code.sh $file
done
