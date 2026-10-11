#!/bin/bash

if [ ! -e .smv_git ]; then
  echo "***error: the script $0 needs to run in the bot/Firebot directory"
  echo "          $0 aborted"
  exit 1
fi

# SMV UG directory
mandir=../../smv/Manuals/SMV_User_Guide/
# smv source directory
smv_dir=../../smv/Source/smokeview

#---------------------------------------------
#                   CHECK_KW
#---------------------------------------------

CHECK_KW ()
{
 local TYPE=$1
 local type=$2
 local FILE=$3
 MANFILE=man_${type}.out
 SMVFILE=smv_${type}.out
 grep Match$TYPE $smv_dir/$FILE              | awk -F'"' '{ print $2}' |                                             sort -u > $SMVFILE
 grep hitem$type $mandir/SMV_User_Guide.tex  | awk -F'{' '{ print $2}' | awk -F'}' '{ print $1}' | sed 's/\\_/_/g' | sort -u > $MANFILE

 echo $type keywords in smokeview source not in smokeview user guide  > smv_$type
 git diff --no-index $SMVFILE $MANFILE  | grep ^- | sed 's/^-//g' >> smv_${type}

 echo $type keywords in smokeview user guide not in smokeview source  > smvug_$type
 git diff --no-index $SMVFILE $MANFILE  | grep ^+ | sed 's/^-//g' >> smvug_${type}

 rm $SMVFILE $MANFILE
}

CHECK_KW INI ini readsmv.c
CHECK_KW SMV smv readsmv.c
CHECK_KW SSF ssf IOscript.c
