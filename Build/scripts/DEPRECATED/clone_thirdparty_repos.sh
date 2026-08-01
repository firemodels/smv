#!/bin/bash

clone_repo()
{
  GITOWNER=$1
  repo=$2
  repo_out=$repo
  
  echo ------------------------------------------------------
  echo ------------------------------------------------------
  echo cloning $repo repo

  GDBUILD=$FMROOT/gd-build
  if [ ! -d $GDBUILD  ]; then
    mkdir $GDBUILD
  fi
  if [ ! -d $GDBUILD/src  ]; then
    mkdir $GDBUILD/src
  fi

  repo_dir=$GDBUILD/src/$repo_out
  if [ -d $repo_dir ]; then
    echo deleting $repo_dir
    rm -rf $repo_dir
  fi
  
  cd $GDBUILD/src
  git clone $GITHEADER$GITOWNER/$repo.git $repo_out
}

CURDIR=`pwd`

cd ../../..
FMROOT=`pwd`
cd $CURDIR

git remote -v | grep origin | head -1 | awk  '{print $2}' | awk -F ':' '{print $1}' > $CURDIR/githeader.out
GITHEADER=`head -1 $CURDIR/githeader.out`

if [ "$GITHEADER" == "git@github.com" ]; then
   GITHEADER="${GITHEADER}:"
   git remote -v | grep origin | head -1 | awk -F ':' '{print $2}' | awk -F\\/ '{print $1}' > $CURDIR/gituser.out
   GITUSER=`head -1 $CURDIR/gituser.out`
else 
   GITHEADER=https://github.com/
   git remote -v | grep origin | head -1 | awk -F '.' '{print $2}' | awk -F\\/ '{print $2}' > $CURDIR/gituser.out
   GITUSER=$CURDIR/gituser.out
fi

rm -f $CURDIR/gituser.out
rm -f $CURDIR/githeader.out

#git clone https://github.com/libjpeg-turbo/libjpeg-turbo.git
#git clone https://github.com/pnggroup/libpng.git
#git clone https://github.com/madler/zlib.git
#git clone https://github.com/libgd/libgd.git

clone_repo libjpeg-turbo  libjpeg-turbo
clone_repo pnggroup       libpng
clone_repo madler         zlib
clone_repo libgd          libgd
