#!/bin/bash
FDSEDITION=FDS6

edition=$1
revision=$2
REMOTESVNROOT=$3
PLATFORMHOST=$4
SVNROOT=~/$5

errlog=/tmp/smv_errlog.$$

TEST=
RELEASE=
if [ "$edition" == "test" ]; then
  TEST=test_
else
  RELEASE=
fi

platform="linux"
platform2="LINUX"
if [ "`uname`" == "Darwin" ]
then
  platform="osx"
  platform2="OSX"
fi

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
    echo "***error: the file $TOFILE failed to copy from $FROMDIR/$FROMFILE">>$errlog
    echo "">>$errlog
  fi
}

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
    echo "***error: the directory $TODIR failed to copy from $FROMDIR">>$errlog
    echo "">$errlog
  fi
}


BACKGROUNDDIR=$REMOTESVNROOT/smv/Build/background/intel_${platform}_64
SMVDIR=$REMOTESVNROOT/smv/Build/smokeview/intel_${platform}_64
SMZDIR=$REMOTESVNROOT/smv/Build/smokezip/intel_${platform}_64
DEM2FDSDIR=$REMOTESVNROOT/smv/Build/dem2fds/intel_${platform}_64
SMDDIR=$REMOTESVNROOT/smv/Build/smokediff/intel_${platform}_64
WIND2FDSDIR=$REMOTESVNROOT/smv/Build/wind2fds/intel_${platform}_64
HASHFILEDIR=$REMOTESVNROOT/smv/Build/hashfile/intel_${platform}_64
FLUSHFILEDIR=$REMOTESVNROOT/smv/Build/flush/intel_${platform}_64
FORBUNDLE=$SVNROOT/smv/Build/Bundle/for_bundle
PLATFORMDIR=$RELEASE$revision\_${platform}64
UPDATER=$SVNROOT/fds/Utilities/Scripts/make_updater.sh
uploads=$SVNROOT/smv/Build/Bundle/uploads
flushfile=$SVNROOT/smv/Build/flulsh/intel_${platform}_64/flush_${platform}_64

cd $uploads

rm -rf $PLATFORMDIR
mkdir -p $PLATFORMDIR
mkdir -p $PLATFORMDIR/bin
mkdir -p $PLATFORMDIR/bin/hash
mkdir -p $PLATFORMDIR/Documentation

echo ""
echo "---- copying files ----"
echo ""
CPDIR $FORBUNDLE/textures $PLATFORMDIR/bin/textures

cp $FORBUNDLE/*.png $PLATFORMDIR/bin/.
cp $FORBUNDLE/*.po $PLATFORMDIR/bin/.

CP $FORBUNDLE objects.svo   $PLATFORMDIR/bin objects.svo
CP $FORBUNDLE smokeview.ini $PLATFORMDIR/bin smokeview.ini
CP $FORBUNDLE volrender.ssf $PLATFORMDIR/bin volrender.ssf

SCP $PLATFORMHOST $BACKGROUNDDIR background                      $PLATFORMDIR/bin background
SCP $PLATFORMHOST $SMVDIR        smokeview_${platform}_${TEST}64 $PLATFORMDIR/bin smokeview
SCP $PLATFORMHOST $DEM2FDSDIR    dem2fds_${platform}_64          $PLATFORMDIR/bin dem2fds
SCP $PLATFORMHOST $SMDDIR        smokediff_${platform}_64        $PLATFORMDIR/bin smokediff
SCP $PLATFORMHOST $SMZDIR        smokezip_${platform}_64         $PLATFORMDIR/bin smokezip
SCP $PLATFORMHOST $WIND2FDSDIR   wind2fds_${platform}_64         $PLATFORMDIR/bin wind2fds
SCP $PLATFORMHOST $HASHFILEDIR   hashfile_${platform}_64         $PLATFORMDIR/bin hashfile
SCP $PLATFORMHOST $FLUSHFILEDIR  flush_${platform}_64            $PLATFORMDIR/bin flush

CURDIR=`pwd`
cd $PLATFORMDIR/bin
hashfile background > background.sha1
hashfile smokediff  > smokediff.sha1
hashfile smokeview  > smokeview.sha1
hashfile smokezip   > smokezip.sha1
hashfile dem2fds    > dem2fds.sha1
hashfile wind2fds   > wind2fds.sha1
hashfile hashfile   > hashfile.sha1
cat *.sha1 > $uploads/$PLATFORMDIR.sha1
cd $CURDIR

rm -f $PLATFORMDIR.tar $PLATFORMDIR.tar.gz
cd $PLATFORMDIR
echo ""
echo "---- building installer ----"
echo ""
tar cvf ../$PLATFORMDIR.tar .
cd ..
gzip $PLATFORMDIR.tar
$UPDATER ${platform2} $revision $PLATFORMDIR.tar.gz $PLATFORMDIR.sh FDS/$FDSEDITION
hashfile $PLATFORMDIR.sh > $PLATFORMDIR.sh.sha1
cat $PLATFORMDIR.sh.sha1 >> $uploads/$PLATFORMDIR.sha1
rm $PLATFORMDIR.sh.sha1

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
