#!/bin/bash

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "Usage:"
echo "$0 [options]"
echo ""
echo "Options:"
echo "-h          - display this message"
exit
}

#---------------------------------------------
#                   is_file_installed
#---------------------------------------------

is_file_installed()
{
  local program=$1

  notfound=`$program -help | tail -1 | grep "not found" | wc -l`
  if [ "$notfound" == "1" ] ; then
    stage2a_success="0"
    echo "***error: $program not installed" >> $OUTPUT_DIR/stage2a
  fi
}

while getopts 'h' OPTION
do
case $OPTION  in
  h)
   usage
   ;;
esac
done
shift $(($OPTIND-1))

# this script is run by ssh

smokeview=$1
DIR=$2
case=$3
renderdir=$4

if [ ! -e $DIR ]; then
  echo "***error: the directory $DIR does not exist"
  exit
fi
if [ "$renderdir" != "" ]; then
  renderdir="-scriptrenderdir $renderdir"
fi

cd $DIR

$smokeview -runhtmlscript $renderdir $case
