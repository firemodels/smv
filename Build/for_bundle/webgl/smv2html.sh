#!/bin/bash

#---------------------------------------------
#                   usage
#---------------------------------------------

function usage {
echo "Usage:"
echo "$0 [options] casename"
echo ""
echo "Options:"
echo "-d dir        - directory containing case"
echo "-e executable - smokeview executable path"
echo "-h            - display this message"
echo "-H hostname   - host name"
echo "-v            - show command that will be run"
exit
}

#---------------------------------------------
#                   is_file_installed
#---------------------------------------------

is_file_installed()
{
  local program=$1
  local host=$2

  if [ "$host" == "" ]; then
    notfound=`$program -h |& tail -1 | grep "not found" | wc -l`
  else
    notfound=`ssh -q $host $program -h |& tail -1 | grep "not found" | wc -l`
  fi
  if [ "$notfound" == "1" ] ; then
    error="1"
    if [ "$host" == "" ]; then
      echo "***error: $program not in path"
    else
      echo "***error: $program not in path on $host"
    fi
  fi
}

DIR=.
hostname=
showcommandline=
SMOKEVIEW=smokeview
renderdir=
error="0"

#*** parse command line options

while getopts 'd:e:hH:r:v' OPTION
do
case $OPTION  in
  d)
   DIR="$OPTARG"
   ;;
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  h)
   usage
   ;;
  H)
   hostname="$OPTARG"
   ;;
  r)
   renderdir="$OPTARG"
   ;;
  v)
   showcommandline=1
   ;;
esac
done
shift $(($OPTIND-1))
casename=$1

thishost=`hostname`
if [[ "$hostname" != "" ]] && [[ "$thishost" != "$hostname" ]]; then
  SSH="ssh -q $hostname "
  HOST=$hostname
else
  SSH=
  HOST=
fi

ECHO=
if [ "$showcommandline" == "1" ]; then
  ECHO="echo "
fi

is_file_installed runsmv_ssh.sh $HOST
is_file_installed $SMOKEVIEW    $HOST

if [ "$error" == "1" ]; then
  exit
fi
echo test

$ECHO $SSH  runsmv_ssh.sh $SMOKEVIEW $DIR $casename $renderdir
