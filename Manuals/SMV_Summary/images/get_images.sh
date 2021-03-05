#!/bin/bash
ROOT=$1

if [ "$ROOT" == "" ]; then
  ROOT=../../../..
fi

cp $ROOT/smv/Manuals/SMV_User_Guide/SCRIPT_FIGURES/*.png         user/.
cp $ROOT/smv/Manuals/SMV_Verification_Guide/SCRIPT_FIGURES/*.png verification/.
