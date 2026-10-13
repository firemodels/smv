#!/bin/bash
MANDIR=$1
MAKEMOVIES=$2

GDRIVE=~/bin/gdrive
CURDIR=`pwd`
# directory containing guides on google drive : FDS-SMV Newest Manuals
MANUAL_PARENT_ID=0B_wB1pJL2bFQUlJwMmNfaHlqME0
EXT=

if [ "$MAKEMOVIES" == "1" ]; then
  EXT=_movies
fi

UPLOADWEB ()
{
  WEBDIR=$1
  WEBDIRtar=${WEBDIR}$EXT.tar
  cd $WEBDIR
  tar cvf ../$WEBDIRtar .
  cd ..
  if [ -e $WEBDIRtar ]; then
    $GDRIVE list  | grep $WEBDIRtar | awk '{ system("~/bin/gdrive delete -i " $1)} '
    $GDRIVE upload -p $MANUAL_PARENT_ID -f $WEBDIRtar
    npubs=`$GDRIVE list  | grep $WEBDIRtar | wc -l`
    if [ $npubs -eq 0 ]; then
      echo "*** warning: The file $WEBDIRtar failed to upload to google drive"
    fi
    if [ $npubs -gt 1 ]; then
      echo "*** warning: More than one copy of $WEBDIRtar exists on google drive"
    fi
    if [ -d $HOME/.smokebot/pubs ]; then
      echo copying $WEBDIRtar to $HOME/.smokebot/pubs
      cp $WEBDIRtar $HOME/.smokebot/pubs/.
    fi
  fi
}

if [ -e $GDRIVE ] ; then
  cd $MANDIR
  UPLOADWEB SMV_Summary
  cd $CURDIR
fi
