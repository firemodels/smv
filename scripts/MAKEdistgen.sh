#!/bin/bash
version=$1
platform=$2
HOST=$3
FDS_EDITION=$4
SVNROOT=$5

MD5=md5sum
if [ "`uname`" == "Darwin" ] ; then
  MD5=md5
fi

errlog=/tmp/smv_errlog.$$

size=64

# ---------------- MDHASH -----------------

MD5HASH ()
{
local DIR=$1
local FILE=$2

local curdir=`pwd`
cd $DIR

$MD5 $FILE
cd $curdir
}

# ---------------- SCP -----------------

SCP ()
{
  HOST=$1
  FROMDIR=$2
  FROMFILE=$3
  TODIR=$4
  TOFILE=$5

  scp $HOST\:$FROMDIR/$FROMFILE $TODIR/$TOFILE 2>/dev/null
  if [ -e $TODIR/$TOFILE ]; then
    echo "$TOFILE copied from $HOST"
  else
    echo "***error: the file $TOFILE failed to copy from: ">>$errlog
    echo "$HOST:$FROMDIR/$FROMFILE">>$errlog
    echo "">>$errlog
  fi
}

# ---------------- CP -----------------

CP ()
{
  FROMDIR=$1
  FROMFILE=$2
  TODIR=$3
  TOFILE=$4
  if [ ! -e $FROMDIR/$FROMFILE ]; then
    echo "***error: the file $FROMFILE does not exist"
  else
    cp $FROMDIR/$FROMFILE $TODIR/$TOFILE
  fi
  if [ -e $TODIR/$TOFILE ]; then
    echo "$TOFILE copied"
  else
    echo "***error: the file $TOFILE failed to copy">>$errlog
    error "">>$errlog
  fi
}

# ---------------- CPDIR -----------------

CPDIR ()
{
  FROMDIR=$1
  TODIR=$2
  if [ ! -e $FROMDIR ]; then
    echo "***error: the directory $FROMDIR does not exist"
  else
    cp -r $FROMDIR $TODIR
  fi
  if [ -e $TODIR ]; then
    echo "$TODIR copied"
  else
    echo "***error: the directory $TODIR failed to copy">>$errlog
    echo "">>$errlog
  fi
}

# ------------------ beginning of script ----------------

platformsize=${platform}_$size
WEBPAGESDIR=$SVNROOT/webpages
BACKGROUNDDIR=$SVNROOT/smv/Build/background/intel_${platform}_64
SMOKEVIEWDIR=$SVNROOT/smv/Build/smokeview/intel_$platformsize
SMOKEZIPDIR=$SVNROOT/smv/Build/smokezip/intel_$platformsize
DEM2FDSDIR=$SVNROOT/smv/Build/dem2fds/intel_$platformsize
SMOKEDIFFDIR=$SVNROOT/smv/Build/smokediff/intel_$platformsize
WINDDIR=$SVNROOT/smv/Build/wind2fds/intel_$platformsize
FORBUNDLE=~/$SVNROOT/smv/for_bundle
DIR=smv_${version}_$platform$size
UPDATER=~/$SVNROOT/fds/Utilities/Scripts/make_updater.sh

cd ~/$SVNROOT/smv/uploads

rm -rf $DIR
mkdir -p $DIR
mkdir -p $DIR/bin
mkdir -p $DIR/bin/MD5
mkdir -p $DIR/Documentation
SCP $HOST $WEBPAGESDIR smv_readme.html $DIR/Documentation release_notes.html

echo ""
echo "--- copying files ---"
echo ""
CPDIR $FORBUNDLE/textures $DIR/bin/textures
CP $FORBUNDLE objects.svo $DIR/bin objects.svo
CP $FORBUNDLE smokeview.ini $DIR/bin smokeview.ini
CP $FORBUNDLE volrender.ssf $DIR/bin volrender.ssf
SCP $HOST $BACKGROUNDDIR background $DIR/bin background
SCP $HOST $SMOKEDIFFDIR smokediff_$platformsize $DIR/bin smokediff
SCP $HOST $SMOKEVIEWDIR smokeview_$platformsize $DIR/bin smokeview
SCP $HOST $SMOKEZIPDIR smokezip_$platformsize $DIR/bin smokezip
SCP $HOST $DEM2FDSDIR dem2fds_$platformsize $DIR/bin dem2fds
SCP $HOST $WINDDIR wind2fds_$platformsize $DIR/bin wind2fds

mdversion=_$version.md5
MD5DIR=$DIR/bin/MD5
MD5HASH $DIR/bin background > $MD5DIR/background$mdversion
MD5HASH $DIR/bin smokediff  > $MD5DIR/smokediff$mdversion
MD5HASH $DIR/bin smokeview  > $MD5DIR/smokeview$mdversion
MD5HASH $DIR/bin smokezip   > $MD5DIR/smokezip$mdversion
MD5HASH $DIR/bin dem2fds    > $MD5DIR/dem2fds$mdversion
MD5HASH $DIR/bin wind2fds   > $MD5DIR/wind2fds$mdversion
cat     $DIR/bin/MD5/*.md5  > $MD5DIR/smv_${version}_${platform}_bundle.md5s

echo ""
echo "--- building installer ---"
echo ""
rm -f $DIR.tar $DIR.tar.gz
cd $DIR
tar cvf ../$DIR.tar .
cd ..
gzip $DIR.tar

platform2=$platform
if [ "$platform" == "linux" ]; then
  platform2=Linux
fi
if [ "$platform" == "osx" ]; then
  platform2=OSX
fi

$UPDATER $platform2 $version $DIR.tar.gz $DIR.sh FDS/$FDS_EDITION

MD5HASH . $DIR.sh > $DIR/bin/MD5/$DIR.sh.md5
cat $DIR/bin/MD5/$DIR.sh.md5 >> $MD5DIR/smv_${version}_${platform}_bundle.md5s

if [ -e $errlog ]; then
  numerrs=`cat $errlog | wc -l `
  if [ $numerrs -gt 0 ]; then
    echo ""
    echo "----------------------------------------------------------------"
    echo "---------------- bundle generation errors ----------------------"
    cat $errlog
    echo "----------------------------------------------------------------"
    echo "----------------------------------------------------------------"
    echo ""
  fi
  rm $errlog
fi

