#!/bin/bash

CURDIR=`pwd`
cd ../../smv/Manuals
MANDIR=`pwd`
cd $CURDIR

TARHOME=$HOME/.smokebot/pubs
if [ ! -e $HOME/.smokebot ]; then
  mkdir $HOME/.smokebot
fi
if [ ! -e $HOME/.smokebot/pubs ]; then
  mkdir $HOME/.smokebot/pubs
fi
tarfile=SMV_Summary.tar
rm -f $TARHOME/$tarfile $TARHOME/${tarfile}.gz
cd $MANDIR
tar cvf $TARHOME/$tarfile SMV_Summary  &> /dev/null
cd $TARHOME
gzip $tarfile
echo ***Uploading $tarfile.gz
gh release upload $GH_SMOKEVIEW_TAG $tarfile.gz -R github.com/$GH_OWNER/$GH_REPO --clobber
