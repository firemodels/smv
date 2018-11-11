#!/bin/bash
PLATFORM=linux64
EXT=.sh
BASEDIR=$HOME/SMVS

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "Download smokeview installers from github"
echo ""
echo "Options:"
echo "-b - directory containing insallers {default: $BASEDIR]"
echo "-h - display usage information"
echo "-l - download and unpack linux installers {default}"
echo "-m - download and unpack mac installers"
echo "-w - download windows installers"
exit
}


GETSMV ()
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
wget -q https://github.com/firemodels/smv/releases/download/$SMVDIR/$SMVFILE
if [ "$EXT" == ".sh" ]; then
  echo extract | bash $SMVFILE > /dev/null
  mkdir $SMVDIR
  cd $SMVDIR
  tar xvf ../$SMV$UNDERSCORE$PLATFORM.tar.gz > /dev/null
  cd ..
fi
}

GETSMV2()
{
SMVDIR=$1
SMV=$2
TAR=$3
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
wget -q https://github.com/firemodels/smv/releases/download/$SMVDIR/$SMVFILE
if [ "$EXT" == ".sh" ]; then
  echo extract | bash $SMVFILE > /dev/null
  mkdir $SMVDIR
  cd $SMVDIR
  tar xvf ../$TAR$UNDERSCORE$PLATFORM.tar.gz > /dev/null
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
GETSMV SMV6.7.1 SMV6.7.1
GETSMV SMV6.7.0 SMV6.7.0

GETSMV SMV6.6.5 SMV6.6.5
GETSMV SMV6.6.4 SMV6.6.4
GETSMV SMV6.6.3 SMV6.6.3
GETSMV2 SMV6.6.2 smv_6.6.2 smv_SMV_6.6.2
GETSMV SMV6.6.1 smv_6.6.1
GETSMV SMV6.6.0 smv_6.6.0

GETSMV SMV6.5.5 smv_6.5.5
GETSMV SMV6.5.4 smv_6.5.4
GETSMV SMV6.5.3 smv_6.5.3
GETSMV SMV6.5.2 smv_6.5.2
GETSMV SMV6.5.1 smv_6.5.1
GETSMV SMV6.5.0 smv_6.5.0

GETSMV SMV6.4.4 smv_6.4.4
GETSMV SMV6.4.3 smv_6.4.3
GETSMV SMV6.4.2 smv_6.4.2
GETSMV SMV6.4.1 smv_6.4.1
GETSMV SMV6.4.0 smv_6.4.0


cd $CURDIR
