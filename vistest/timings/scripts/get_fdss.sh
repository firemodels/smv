#!/bin/bash
PLATFORM=linux64
EXT=.sh
BASEDIR=$HOME/FDSS

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "Download fds-smv installers from github"
echo ""
echo "Options:"
echo "-b - directory containing installers {default: $BASEDIR]"
echo "-h - display usage information"
echo "-l - download and unpack linux installers {default}"
echo "-m - download and unpack mac installers"
echo "-w - download windows installers"
exit
}


GETFDS ()
{
GITDIR=$1
FDSDIR=$2
FDS=$3
UNDERSCORE=_
FDSINSTALLER=$FDS$UNDERSCORE$PLATFORM$EXT
FDSTAR=$FDS$UNDERSCORE$PLATFORM.tar.gz

echo downloading $PLATFORM installer for $FDS
if [ -e $FDSINSTALLER ]; then
  rm -f $FDSINSTALLER
fi
wget -q https://github.com/firemodels/fds/releases/download/$GITDIR/$FDSINSTALLER
if [ "$EXT" == ".sh" ]; then
  echo "  extracting tar file"
  if [ -e $FDSTAR ]; then
    rm -f $FDSTAR
  fi
  echo extract | bash $FDSINSTALLER > /dev/null
  if [ -d $FDSDIR ]; then
    rm -rf $FDSDIR
  fi
  mkdir $FDSDIR
  cd $FDSDIR
  echo "  untarring"
  tar xvf ../$FDSTAR > /dev/null
  echo ""
  cd ..
fi
}

while getopts 'b:hlmw' OPTION
do
case $OPTION  in
  b)
   BASEDIR="$OPTARG"
   ;;
  h)
   usage
   ;;
  l)
   PLATFORM="linux64"
   EXT=".sh"
   ;;
  m)
   PLATFORM="osx64"
   EXT=".sh"
   ;;
  w)
   PLATFORM="win64"
   EXT=".exe"
   ;;
esac
done
shift $(($OPTIND-1))

if [ ! -d $BASEDIR ]; then
  mkdir -p $BASEDIR
  if [ ! -d $BASEDIR ]; then
    echo "fatal error: unable to create directory $BASEDIR"
    exit
  fi
fi

CURDIR=`pwd`
cd

if [ ! -d $BASEDIR/$PLATFORM ]; then
  mkdir -p $BASEDIR/$PLATFORM
fi

cd $BASEDIR/$PLATFORM
GETFDS FDS6.7.0 FDS6.7.0 FDS6.7.0-SMV6.7.1
GETFDS FDS6.6.0 FDS6.6.0 FDS_6.6.0-SMV_6.6.0
GETFDS FDS6.5.3 FDS6.5.3 FDS_6.5.3-SMV_6.4.4
GETFDS Git-r21  FDS6.5.2 FDS_6.5.2-SMV_6.3.12
GETFDS Git-r16  FDS6.5.1 FDS_6.5.1-SMV_6.3.9
GETFDS Git-r14  FDS6.5.0 FDS_6.5.0-SMV_6.3.8
GETFDS Git-r10  FDS6.4.0 FDS_6.4.0-SMV_6.3.6
GETFDS Git-r5   FDS6.3.2 FDS_6.3.2-SMV_6.3.2
GETFDS Git-r4   FDS6.3.1 FDS_6.3.1-SMV_6.3.2
GETFDS Git-r1   FDS6.3.0 FDS_6.3.0-SMV_6.3.0

cd $CURDIR
