#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ "`which firefox`" == "" ]; then
  echo "***error: web browser, firefox, is not installed"
  exit
fi

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR/../../..
ROOT_DIR=`pwd`

SUMMARY_DIR=$ROOT_DIR/smv/Manuals/SMV_Summary
SUMMARY_FILE=$SUMMARY_DIR/diffs.html
if [ -e $SUMMARY_FILE ]; then
  firefox $SUMMARY_FILE >& /dev/null
else
  echo "***error: $SUMMARY_FILE does not exist"
fi
