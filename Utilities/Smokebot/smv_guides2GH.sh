#!/bin/bash

CURDIR=`pwd`

PUBSDIR=$HOME/.smokebot/pubs_latest

cd ../../smv/Manuals
MANDIR=`pwd`

cd $HOME/.smokebot/apps_latest
APPSDIR=`pwd`

cd $CURDIR

UPLOADINFO ()
{
  DIR=$HOME/.smokebot/apps
  S_HASH=`head -1     $DIR/SMV_HASH`
  S_REVISION=`head -1 $DIR/SMV_REVISION`
  echo "SMV_HASH          $S_HASH" > $DIR/SMV_INFO.txt
  echo "SMV_REVISION $S_REVISION" >> $DIR/SMV_INFO.txt
  echo ***Uploading SMV_INFO.txt
  gh release upload $GH_SMOKEVIEW_TAG $DIR/SMV_INFO.txt -R github.com/$GH_OWNER/$GH_REPO --clobber
}

UPLOADGUIDE ()
{
  FILE=$1
  FILEnew=${FILE}.pdf
  if [ -e $PUBSDIR/$FILEnew ]; then
    echo ***Uploading $FILEnew
    gh release upload $GH_SMOKEVIEW_TAG $PUBSDIR/$FILEnew -R github.com/$GH_OWNER/$GH_REPO --clobber
  else
    echo ***error: $PUBSDIR/$FILEnew not found
  fi
}

UPLOADFIGURES ()
{
  DIRECTORY=$1
  FILE=$2
  TARHOME=$HOME/.smokebot/pubs
  if [ ! -e $HOME/.smokebot ]; then
    mkdir $HOME/.smokebot
  fi
  if [ ! -e $HOME/.smokebot/pubs ]; then
    mkdir $HOME/.smokebot/pubs
  fi
  cd $MANDIR/$DIRECTORY/SCRIPT_FIGURES
  tarfile=${FILE}_figures.tar
  rm -f $TARHOME/$tarfile
  rm -f $TARHOME/$tarfile.gz
  tar cvf $TARHOME/$tarfile . &> /dev/null
  cd $TARHOME
  gzip $tarfile
  echo ***Uploading $tarfile.gz
  gh release upload $GH_SMOKEVIEW_TAG $TARHOME/$tarfile.gz -R github.com/$GH_OWNER/$GH_REPO --clobber
}

UPLOADGUIDE SMV_Technical_Reference_Guide
UPLOADGUIDE SMV_User_Guide
UPLOADGUIDE SMV_Verification_Guide
UPLOADFIGURES SMV_User_Guide SMV_UG
UPLOADFIGURES SMV_Verification_Guide SMV_VG
UPLOADFIGURES SMV_Technical_Reference_Guide SMV_TG
UPLOADINFO
