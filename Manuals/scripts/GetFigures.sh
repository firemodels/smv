#!/bin/bash
GH_OWNER=firemodels
GH_REPO=test_bundles

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "Usage:"
echo "GetFigures.sh options"
echo ""
echo "Download FDS and/or Smokeview manual figures from github and copy to their"
echo "respecitve manual SCRIPT_DIRS directories"
echo ""
echo "Options:"
echo "-a - copy all FDS and Smokeview manual figures"
echo "-f - copy FDS figures for manuals specified by -u, -v, -V and or -t options"
echo "-F - copy FDS figures for all guides"
echo "-h - display this message"
echo "-f - copy Smokeview figures for manuals specified by -u and/or -v options"
echo "-S - copy Smokeview figures for both user and verification guides"
echo "-t - copy FDS technical guide figures"
echo "-u - copy user guide figures  (also specify either -f or -s)"
echo "-v - copy verification guide figures  (also specify either -f or -s)"
echo "-V - copy FDS validation guide figures"
exit 0
}

#*** parse command line options

while getopts 'afFhsStuvV' OPTION
do
case $OPTION  in
  a)
  FDS=1
  SMV=1
  USER=1
  VER=1
  VAL=1
  TECH=1
  ;;
  f)
  FDS=1
  ;;
  F)
  FDS=1
  USER=1
  VER=1
  VAL=1
  TECH=1
  ;;
  h)
   usage;
   ;;
  s)
  SMV=1
  ;;
  S)
  SMV=1
  USER=1
  VER=1
  ;;
  t)
  FDS=1
  TECH=1
  ;;
  u)
  USER=1
  ;;
  v)
  VER=1
  ;;
  V)
  FDS=1
  VAL=1
  ;;
  \?)
  echo "***error: unknown option entered. aborting firebot"
  exit 1
  ;;
esac
done
shift $(($OPTIND-1))

if [[ "$FDS" == "" ]] && [[ "$SMV" == "" ]]; then
  FDS=1
fi

if [ "SMV" != "" ]; then
  if [[ "$USER" == "" ]] && [[ "$VER" == "" ]]; then
    USER=1
  fi
fi

if [ "FDS" != "" ]; then
  if [[ "$USER" == "" ]] && [[ "$VER" == "" ]] && [[ "$VAL" == "" ]] && [[ "$TECH" == "" ]]; then
    USER=1
  fi
fi

CURDIR=`pwd`

cd files
FILESDIR=`pwd`

echo cleaning $FILESDIR
git clean -dxf >& /dev/null

cd $CURDIR

DOWNLOADFILE ()
{
  TAG=$1
  FILE=$2
  echo ""
  echo downloading $FILE
  gh release download $TAG -p $FILE -D $FILESDIR  -R github.com/$GH_OWNER/$GH_REPO
}

COPYFILES ()
{
  TODIR=$1
  FILE=$2
  if [ -d $TODIR ]; then
    echo "untarring $FILE to $TODIR"
    cd $TODIR
    if [ -e $FILESDIR/$FILE ]; then
      tar xf $FILESDIR/$FILE > /dev/null 2>&1
    else
      echo "***error: $FILESDIR/$FILE does not exist"
    fi
  else
    echo ***error: $TODIR does not exist
  fi
}

SMVREPO=../../smv
cd $SMVREPO
SMVREPO=`pwd`
cd $CURDIR

if [ "$SMV" != "" ]; then
  if [ "$USER" != "" ]; then
    DOWNLOADFILE SMOKEVIEW_TEST SMV_UG_figures.tar.gz
    COPYFILES $SMVREPO//Manuals/SMV_User_Guide/SCRIPT_FIGURES        SMV_UG_figures.tar.gz
  fi

  if [ "$VER" != "" ]; then
    DOWNLOADFILE SMOKEVIEW_TEST SMV_VG_figures.tar.gz
    COPYFILES $SMVREPO/Manuals/SMV_Verification_Guide/SCRIPT_FIGURES SMV_VG_figures.tar.gz
  fi
fi

cd $CURDIR
FDSREPO=../../fds
cd $FDSREPO
FDSREPO=`pwd`
cd $CURDIR

if [ "$FDS" != "" ]; then
  if [ "$USER" != "" ]; then
    DOWNLOADFILE  FDS_TEST FDS_UG_figures.tar.gz
    COPYFILES $FDSREPO/Manuals/FDS_User_Guide/SCRIPT_FIGURES                FDS_UG_figures.tar.gz
  fi

  if [ "$TECH" != "" ]; then
    DOWNLOADFILE  FDS_TEST FDS_TG_figures.tar.gz
    COPYFILES $FDSREPO/Manuals/FDS_Technical_Reference_Guide/SCRIPT_FIGURES FDS_TG_figures.tar.gz
  fi

  if [ "$VER" != "" ]; then
    DOWNLOADFILE  FDS_TEST FDS_VERG_figures.tar.gz
    COPYFILES $FDSREPO/Manuals/FDS_Verification_Guide/SCRIPT_FIGURES        FDS_VERG_figures.tar.gz
  fi

  if [ "$VAL" != "" ]; then
    DOWNLOADFILE  FDS_TEST FDS_VALG_figures.tar.gz
    COPYFILES $FDSREPO/Manuals/FDS_Validation_Guide/SCRIPT_FIGURES          FDS_VALG_figures.tar.gz
  fi
fi
cd $CURDIR
exit 0
