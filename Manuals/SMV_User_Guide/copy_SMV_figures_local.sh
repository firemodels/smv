#!/bin/bash
ROOTDIR=$1

CURDIR=`pwd`
FROM_SMVVGDIR=$ROOTDIR/SMV_Verification_Guide/SCRIPT_FIGURES
FROM_SMVUGDIR=$ROOTDIR/SMV_User_Guide/SCRIPT_FIGURES

cd SCRIPT_FIGURES
TO_SMVUGDIR=`pwd`

cd $CURDIR
cd ../SMV_Verification_Guide/SCRIPT_FIGURES
TO_SMVVGDIR=`pwd`

cd $CURDIR

if [ -d $FROM_SMUVGDIR ]; then
  cp $FROM_SMVUGDIR/* $TO_SMVUGDIR/.
  echo User Guide figures copied
else
  echo $FROM_SMVUGDIR does not exist
fi   
if [ -d $FROM_SMVVGDIR ]; then
  cp $FROM_SMVVGDIR/* $TO_SMVVGDIR/.
  echo Verification Guide figures copied
else
  echo $FROM_SMVVGDIR does not exist
fi   
