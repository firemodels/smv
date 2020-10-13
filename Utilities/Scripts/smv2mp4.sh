#!/bin/bash

#---------------------------------------------
#                   Usge
#---------------------------------------------

function Usage {
  scriptname=`basename $0`
  echo "Usage: $scriptname [options] casename"
  echo ""
  echo "This script generates image frames and an mp4 animation of an fds case"
  echo ""
  echo "-e path - full path of smokeview executable."
  echo "     [default: $SMOKEVIEW]"
  echo "-h - show this mesage"
  echo "-i - use installed smokeview"
  exit
}

#---------------------------------------------
#                   is_smokeview_installed
#---------------------------------------------

is_smokeview_installed()
{
  out=/tmp/program.out.$$
  smokeview -v >& $out
  notfound=`cat $out | tail -1 | grep "not found" | wc -l`
  rm $out
  if [ "$notfound" == "1" ] ; then
    echo "***error: smokeview is not installed.  Add it to your PATH or"
    echo "          build the smv repo version of smokeview and use it"
    return 1
  fi
  return 0
}

#---------------------------------------------
#                   CHECK_WRITE
#---------------------------------------------

CHECK_WRITE ()
{
  DIR=$1
  if [ ! -e $DIR ]; then
    mkdir $DIR
    if [ ! -e $DIR ]; then
      echo "***error: the directory $DIR could not be created"
      return 1
    fi
  fi
  touch $DIR/.test
  if [ ! -e $DIR/.test ]; then
    echo "***error: the directdory $DIR cannot be written too"
    return 1
  fi
  rm $DIR/.test
}


#---------------------------------------------
#                   wait_cases_end
#---------------------------------------------

wait_cases_end()
{
  while [[ `qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep ${JOBPREFIX} | grep -v 'C$'` != '' ]]; do
     JOBS_REMAINING=`qstat -a | awk '{print $2 $4 $10}' | grep $(whoami) | grep ${JOBPREFIX} | grep -v 'C$' | wc -l`
     echo "Waiting for ${JOBS_REMAINING} cases to complete."
     sleep 15
  done
}

#---------------------------------------------
#                   restore_state
#---------------------------------------------

restore_state()
{
  if [ -e $GLOBALCONFIG ]; then
    source $GLOBALCONFIG
    NPROCS=${FDS2MOV_NPROCS}
    QUEUE=${FDS2MOV_QUEUE}
    RENDERDIR=${FDS2MOV_RENDERDIR}
    MOVIEDIR=${FDS2MOV_MOVIEDIR}
    EMAIL=${FDS2MOV_EMAIL}
  fi
}

#---------------------------------------------
#                   save_state
#---------------------------------------------

save_state()
{
  echo "#/bin/bash"                           >  $GLOBALCONFIG
  echo "export FDS2MOV_NPROCS=$NPROCS"        >> $GLOBALCONFIG
  echo "export FDS2MOV_QUEUE=$QUEUE"          >> $GLOBALCONFIG
  echo "export FDS2MOV_RENDERDIR=$RENDERDIR"  >> $GLOBALCONFIG
  echo "export FDS2MOV_MOVIEDIR=$MOVIEDIR"    >> $GLOBALCONFIG
  echo "export FDS2MOV_EMAIL=$EMAIL"          >> $GLOBALCONFIG
  
  LOCALCONFIG=$CONFIGDIR/fds2mp4_${input}
  echo "#/bin/bash"                               >  $LOCALCONFIG
}

#---------------------------------------------
#                  generate_images_movie
#---------------------------------------------

generate_images_movie ()
{
while true; do
  echo ""
  echo "     processes: $NPROCS"
  echo "         queue: $QUEUE"
  echo "       mp4 dir: $MOVIEDIR"
  echo "       png dir: $RENDERDIR"
  echo "     smokeview: $SMOKEVIEW"
  echo "       qsmv.sh: $QSMV"
  echo "  image script: $img_scriptname"
  echo "         email: $EMAIL"
  echo ""
  echo "a - define directory containing animation"
  echo "p - define number of processes"
  echo "q - define queue"
  echo "r - define directory containing rendered images"
  echo "m - select emaail address"
  echo "1 - generate PNG images"
  echo "2 - generate PNG images and an MP4 animation"
  echo "x - exit"
  read -p "option: " ans
  if [ "$ans" == "a" ]; then
    read -p "   enter animation directory: " MOVIEDIR
    CHECK_WRITE $MOVIEDIR
    continue
  fi
  if [ "$ans" == "i" ]; then
    read -p "   enter image frame directory: " RENDERDIR
    CHECK_WRITE $RENDERDIR
    continue
  fi
  if [ "$ans" == "m" ]; then
    read -p "   enter email address: " EMAIL
    continue
  fi
  if [ "$ans" == "p" ]; then
    read -p "   enter number of processes: " NPROCS
    continue
  fi
  if [ "$ans" == "q" ]; then
    read -p "   enter queue: " QUEUE
    continue
  fi
  if [ "$ans" == "x" ]; then
    save_state
    exit
  fi
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "2" ]]; then
    if [ "$ans" == "1" ]; then
      GENERATE_IMAGES=1
      GENERATE_SCRIPTS
      return
    fi
    if [ "$ans" == "2" ]; then
      GENERATE_IMAGES=1
      MAKE_MOVIE=1
      GENERATE_SCRIPTS
      return
    fi
  fi
done
}

#---------------------------------------------
#                   GENERATE_SCRIPT
#---------------------------------------------

GENERATE_SCRIPTS ()
{
  cat << EOF > ${smv_scriptname}
RENDERDIR
  $RENDERDIR
UNLOADALL
RENDERALL
  0
  $img_basename 
  0 1
EOF
  echo ""
cat << EOF > $img_scriptname
#!/bin/bash
NPROCS=$NPROCS
QUEUE=$QUEUE
SMOKEVIEW=$SMOKEVIEW
QSMV=$FIREMODELS/smv/Utilities/Scripts/qsmv.sh
\$QSMV -j $JOBPREFIX -P \$NPROCS -q \$QUEUE -e \$SMOKEVIEW -c $smv_scriptname $input
EOF
chmod +x $img_scriptname
}

#----------------------- beginning of script --------------------------------------

#*** initialize variables

RENDERDIR=.
MOVIEDIR=/var/www/html/`whoami`
if [ ! -e $MOVIEDIR ]; then
  MOVIEDIR=.
fi
NPROCS=20
QUEUE=batch4
JOBPREFIX=SV_
GENERATE_IMAGES=
MAKE_MOVIE=

CONFIGDIR=$HOME/.smokeview
if [ ! -e $CONFIGDIR ]; then
  mkdir $CONFIGDIR
fi
GLOBALCONFIG=$CONFIGDIR/smv2mp4_global

# define repo variables

CURDIR=`pwd`
SCRIPTDIR=`dirname "$0"`
cd $SCRIPTDIR/../../..
ROOTDIR=`pwd`
SMVREPO=$ROOTDIR/smv
cd $CURDIR
SMOKEVIEW=$SMVREPO/Build/smokeview/intel_linux_64/smokeview_linux_64
QSMV=$SMVREPO/Utilities/Scripts/qsmv.sh
MAKEMOVIE=$SMVREPO/Utilities/Scripts/make_movie.sh
EMAIL=
smv_scriptname=


#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'e:his:' OPTION
do
case $OPTION  in
  e)
   SMOKEVIEW="$OPTARG"
   ;;
  h)
   Usage
   exit
   ;;
  i)
   is_smokeview_installed || exit 1
   SMOKEVIEW=`which smokeview`
   ;;
esac
done
shift $(($OPTIND-1))

input=$1
img_scriptname=$1.sh
img_basename=$1
restore_state

smv_scriptname=$1.ssf

smvfile=$1.smv

abort=
if [ ! -e $SMOKEVIEW ]; then
  echo "***error: smokeview not found at $SMOKEVIEW"
  abort=1
fi
if [ ! -e $smvfile ]; then
  echo "***error: $smvfile does not exist"
  abort=1
fi
if [ "$abort" == "1" ]; then
  exit
fi

generate_images_movie

save_state

if [ "$GENERATE_IMAGES" == "1" ]; then
  bash $img_scriptname
  if [ "$MAKE_MOVIE" == "1" ]; then
    wait_cases_end
    animation_file=$MOVIEDIR/${img_basename}.mp4
    $MAKEMOVIE -i $RENDERDIR -o $MOVIEDIR $img_basename $img_basename
    if [ "$EMAIL" != "" ]; then
      if [ -e $animation_file ]; then
        echo "animation file, $animation_file, sent to $EMAIL"
        echo "" | mail -s "$img_basename animation" -a $animation_file $EMAIL
      fi
    fi
  fi
else
  echo ""
  echo "image generatingscript: $img_scriptnme"
  cat $img_scriptname
fi

