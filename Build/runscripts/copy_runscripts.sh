#!/bin/bash
todir=$HOME/bin
if [ ! -d $todir ]; then
  echo creating $todir
  echo add $todir to your PATH variable
  mkdir $todir
fi
cp fds*.sh $todir/.
cp smv*.sh $todir/.
