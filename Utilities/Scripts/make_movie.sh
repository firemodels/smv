#!/bin/bash
if [ $# -lt 1 ] ; then
  echo "Usage: makemovie.sh [-i input_directory] [-o output_directory] [-m movie_name] base_name"
  echo ""
  echo "This script generates a movie from a sequence of "
  echo "png image files.  Each image file has the form base_namexxxx.png"
  echo "where xxxx is a frame number."
  echo ""
  echo "-i dir - directory where movie frames are located (default: .)"
  echo "-o dir - directory where movie will be placed (default: .)"
  echo "-m movie name - name of movie generated (default: input_base.mp4)"
  echo ""
  exit
fi

indir=.
outdir=.
moviename=
while getopts 'i:o:m:' OPTION
do
case $OPTION in
  i)
  indir="$OPTARG"
  ;;
  o)
  outdir="$OPTARG"
  ;;
  m)
  moviename="$OPTARG"
  ;;
esac
done
shift $((OPTIND-1))

CURDIR=`pwd`

cd $outdir
outdir=`pwd`

#create movie file name
base=$1
underscore=_
EXT=.mp4
if [ "$moviename" == "" ] ; then
  moviename=$base$EXT
else
  moviename=$moviename$EXT
fi

echoerr() { echo "$@" 1>&2; }

# make sure ffmpeg exists
ffmpeg -h  >& /tmp/ffmpeg.out.$$
ffmpeg_not_found=`cat /tmp/ffmpeg.out.$$ | grep 'not found' | wc -l`
if [ "$ffmpeg_not_found" == "1" ]; then
  echoerr "***error: ffmpeg not found."
  echoerr "          generation of $moviename aborted"
  exit 1
fi

#create movie

cd $CURDIR
cd $indir

echoerr Creating the movie file $outdir/$moviename
ffmpeg -y -r 30 -i $base$underscore%04d.png -vcodec libx264 -pix_fmt yuv420p $outdir/$moviename
if [ -e $outdir/$moviename ]; then
  echoerr The movie file $outdir/$moviename was created.
else
  echoerr "***error ffmpeg failed to create the movie file $outdir/$moviename"
fi
