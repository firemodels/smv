#!/bin/bash
while getopts 'Ad:np:t' OPTION
do
case $OPTION  in
  A)
   dummy=
   ;;
  d)
   DIR="$OPTARG"
   ;;
  n)
   dummy=
   ;;
  p)
   dummy="$OPTARG"
   ;;
  t)
   dummy=
   ;;
esac
done
shift $(($OPTIND-1))

case=$1
cd $DIR
