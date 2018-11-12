#!/bin/bash
PLATFORM=linux64
EXT=.sh
BASEDIR=$HOME/SMVS

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "Download fds-smv installers from github"
echo ""
echo "Options:"
echo "-b - directory containing intsallers {default: $BASEDIR]"
echo "-h - display usage information"
echo "-l - download and unpack linux installers {default}"
echo "-m - download and unpack mac installers"
echo "-w - download windows installers"
exit
}


GETFDS ()
{
SMVDIR=$1
SMV=$2
UNDERSCORE=_
SMVFILE=$SMV$UNDERSCORE$PLATFORM$EXT
if [ "$EXT" == ".sh" ]; then
  echo downloading and unpacking $SMVFILE
else
  echo downloading $SMVFILE
fi
if [ -e $SMVFILE ]; then
  rm -f $SMVFILE 
fi
wget -q https://github.com/firemodels/fds/releases/download/$SMVDIR/$SMVFILE
if [ "$EXT" == ".sh" ]; then
  echo extract | bash $SMVFILE > /dev/null
  mkdir $SMVDIR
  cd $SMVDIR
  tar xvf ../$SMV$UNDERSCORE$PLATFORM.tar.gz > /dev/null
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
GETFDS FDS6.7.0 FDS6.7.0-SMV6.7.1
GETFDS FDS6.6.0 FDS_6.6.0-SMV_6.6.0
GETFDS FDS6.5.3 FDS_6.5.3-SMV_6.4.4
GETFDS FDS6.5.2 FDS_6.5.2-SMV_6.3.12
GETFDS FDS6.5.1 FDS_6.5.1-SMV_6.3.9
GETFDS FDS6.5.0 FDS_6.5.0-SMV_6.3.8
GETFDS FDS6.3.2 FDS_6.3.2-SMV_6.3.2
GETFDS FDS6.3.1 FDS_6.3.1-SMV_6.3.2
GETFDS FDS6.3.0 FDS_6.3.0-SMV_6.3.0

cd $CURDIR
