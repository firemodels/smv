#!/bin/bash

curdir=`pwd`
cd ../..
reporoot=`pwd`
dir1=$reporoot/fds/Utilities/Scripts
dir2=$reporoot/fds/Build/Scripts
dir3=$reporoot/fds/Verification/scripts
dir4=$reporoot/smv/scripts
dir5=$reporoot/bot/Firebot
dir6=$reporoot/bot/Smokebot
h1=$reporoot/bot/Bundle/build_bundle.html
h2=$reporoot/fds/Build/build_fds.html
h3=$reporoot/smv/Build/build_smokeview.html
h4=$reporoot/smv/Build/build_guides.html

cd $curdir

#FILES="$dir3/*.sh $dir3/*.bat"
FILES="$dir1/*.sh $dir1/*.bat"
for f in $FILES
do
file="${f##*/}"
file2=$file
file="${file%.*}"
numhits=`grep -i $file $dir1/*.sh $dir1/*.bat $dir2/*.bat $dir2/*.sh $dir3/*.sh $dir3/*.bat $dir4/*.sh $dir4/*.bat $dir5/*.bat $dir5/*.sh $dir6/*.bat $dir6/*.sh $h1 $h2 $h3 $h4 | wc -l`
if [ "$numhits" == "0" ]; then
echo file=$file2 hits=$numhits
fi
done
