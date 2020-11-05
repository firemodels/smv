#!/bin/bash

#---------------------------------------------
#                   Usge
#---------------------------------------------

function Usage {
  scriptname=`basename $0`
  echo "Usage: $scriptname [options] casename"
  echo ""
  echo "This script generates image frames and an mp4 animation for a slice file"
  echo ""
  echo "-e path - full path of smokeview executable."
  echo "     [default: $SMOKEVIEW]"
  echo "-h - show this message"
  echo "-i - use installed smokeview"
  echo "-O - only output frame from the last smokeview intance (debug option)"
  echo "-v - show but do not run image movie generating scripts"
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
#                   OUTPUT_VIEWPOINTS
#---------------------------------------------

OUTPUT_VIEWPOINTS ()
{
  cat $viewpointmenu | awk -F"," '{ print $1" ",$2}'
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
#                   OUTPUT_SLICES
#---------------------------------------------

OUTPUT_SLICES ()
{
  cat $slicefilemenu | awk -F"," '{ print $1" ",$2," ",$3," ",$4}'
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
    SHARE=${FDS2MOV_SHARE}
  fi
  LOCALCONFIG=$CONFIGDIR/fds2mp4_${input}
  if [ -e $LOCALCONFIG ]; then
    source $LOCALCONFIG
    viewpoint=$FDS2MOV_VIEWPOINT
    viewpointd=$FDS2MOV_VIEWPOINTD
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
  echo "export FDS2MOV_SHARE=$SHARE"          >> $GLOBALCONFIG
  
  LOCALCONFIG=$CONFIGDIR/fds2mp4_${input}
  echo "#/bin/bash"                                  >  $LOCALCONFIG
  echo "export FDS2MOV_VIEWPOINT=\"$viewpoint\""    >> $LOCALCONFIG
  echo "export FDS2MOV_VIEWPOINTD=\"$viewpointd\""  >> $LOCALCONFIG
}

#---------------------------------------------
#                  generate_images
#---------------------------------------------

select_options ()
{
while true; do
  echo ""
  echo "slice quantity: $slice_quantity/$slice_dir=$slice_pos "
  echo "       mp4 dir: $MOVIEDIR"
  echo "       PNG dir: $RENDERDIR"
  echo "     smokeview: $SMOKEVIEW"
  echo "       qsmv.sh: $QSMV"
if [ "$viewpointd" != "" ]; then
  echo "     viewpoint: $viewpointd"
else
  echo "     viewpoint: $viewpoint"
fi
  echo "     processes: $NPROCS"
  if [ "$SHARE" == "" ]; then
    echo "  node sharing: off"
  else
    echo "  node sharing: on"
  fi
  echo "         queue: $QUEUE"
  echo "  image script: $img_scriptname"
  echo "         email: $EMAIL"
  echo ""
  echo "s - select slice"
  echo "a - set animation directory"
  echo "r - set rendered iamge directory "
  echo "v - select viewpoint"
  echo "m - set email address"
  echo ""
  echo "p - set number of processes"
  echo "S - turn on/off node sharing"
  echo "q - set queue"
  echo ""
  echo "1 - generate MP4 animation"
  echo "x - exit"
  read -p "option: " ans
  if [ "$ans" == "a" ]; then
    read -p "   enter animation directory: " MOVIEDIR
    CHECK_WRITE $MOVIEDIR
    continue
  fi
  if [ "$ans" == "r" ]; then
    read -p "   enter image frame directory: " RENDERDIR
    CHECK_WRITE $RENDERDIR
    continue
  fi
  if [ "$ans" == "s" ]; then
    select_slice_file
    continue
  fi
  if [ "$ans" == "S" ]; then
    if [ "$SHARE" == "" ]; then
      SHARE="-T"
    else
      SHARE=""
    fi
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
  if [ "$ans" == "v" ]; then
    select_viewpoint
    continue
  fi
  if [ "$ans" == "x" ]; then
    save_state
    exit
  fi
  if [ "$ans" == "1" ]; then
    GENERATE_SCRIPTS $slice_index
    make_movie
  fi
done
}

#---------------------------------------------
#                   select_viewpoint
#---------------------------------------------

select_viewpoint ()
{
while true; do
  OUTPUT_VIEWPOINTS
  read -p "Select viewpoint: " ans
  if [ "$ans" == "d" ]; then
    viewpoint=
    return 0
  fi
  if [ "$ans" == "x" ]; then
    viewpoint=
    viewpointd="VIEWXMIN"
    return 0
  fi
  if [ "$ans" == "X" ]; then
    viewpoint=
    viewpointd="VIEWXMAX"
    return 0
  fi
  if [ "$ans" == "y" ]; then
    viewpoint=
    viewpointd="VIEWYMIN"
    return 0
  fi
  if [ "$ans" == "Y" ]; then
    viewpoint=
    viewpointd="VIEWYMAX"
    return 0
  fi
  if [ "$ans" == "z" ]; then
    viewpoint=
    viewpointd="VIEWZMIN"
    return 0
  fi
  if [ "$ans" == "Z" ]; then
    viewpoint=
    viewpointd="VIEWZMAX"
    return 0
  fi
  re='^[0-9]+$'
  if ! [[ $ans =~ $re ]]; then
    echo "***error: $ans is an invalid selection"
    continue
  fi
  if [[ $ans -ge 1 ]] && [[ $ans -le $nviewpoints ]]; then
    viewpoint_index=$ans
    viewpoint=`cat $viewpointmenu | awk -v ind="$viewpoint_index" -F"," '{ if($1 == ind){print $2} }'`
    return 0
  else
    echo index $ans out of bounds
  fi
done
}

#---------------------------------------------
#                   select_slice_file
#---------------------------------------------

select_slice_file ()
{
while true; do
  OUTPUT_SLICES
  re='^[0-9]+$'
  read -p "Select slice file: " ans
  if ! [[ $ans =~ $re ]]; then
    echo "***error: $ans is an invalid selection"
    continue
  fi
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    slice_index=$ans
    img_basename=${input}_slice_${slice_index}
    smv_scriptname=$SMVSCRIPTDIR${img_basename}.ssf
    img_scriptname=$SMVSCRIPTDIR${img_basename}.sh
    slice_quantity=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $2} }'`
    slice_quantity=`trim "$slice_quantity"`
    slice_dir=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $3} }'`
    slice_pos=`cat $slicefilemenu | awk -v ind="$slice_index" -F"," '{ if($1 == ind){print $4} }'`
    slice_dir=$(echo $slice_dir | tr -d ' ')
    slice_pos=$(echo $slice_pos | tr -d ' ')
    if [ "$slice_dir" == "1" ]; then
      slice_dir="X"
    fi
    if [ "$slice_dir" == "2" ]; then
      slice_dir="Y"
    fi
    if [ "$slice_dir" == "3" ]; then
      slice_dir="Z"
    fi
    return 0
  else
    echo index $ans out of bounds
  fi
done
}

#---------------------------------------------
#                   trim
#---------------------------------------------

trim()
{
  local var="$*"
# remove leading whitespace characters
  var="${var#"${var%%[![:space:]]*}"}"
# remove trailing whitespace characters
  var="${var%"${var##*[![:space:]]}"}"   
  printf '%s' "$var"
}

#---------------------------------------------
#                   make_movie
#---------------------------------------------

make_movie() {


  if [ "$v_opt" != "" ]; then
    echo ""
    echo "image generatingscript: $img_scriptnme"
    cat $img_scriptname
    return
  fi

  bash $img_scriptname
  wait_cases_end

  nerrs=`grep Error ${input}_f*_s*.err | wc -l`
  if [ "$nerrs" != "0" ]; then 
    grep Error ${input}_f*_s*.err | tail
  else
    animation_file=$MOVIEDIR/${img_basename}.mp4
    $MAKEMOVIE -i $RENDERDIR -o $MOVIEDIR $img_basename $img_basename
    if [ "$EMAIL" != "" ]; then
      if [ -e $animation_file ]; then
        echo "animation file, $animation_file, sent to $EMAIL"
        echo "" | mail -s "animation of $slice_quantity" -a $animation_file $EMAIL
      fi
    fi
  fi
}

#---------------------------------------------
#                   GENERATE_SCRIPT
#---------------------------------------------

GENERATE_SCRIPTS ()
{
  ind=$1
  cat << EOF > ${smv_scriptname}
RENDERDIR
  $RENDERDIR
UNLOADALL
LOADINIFILE
 ${input}.ini
EOF
if [ "$viewpointd" != "" ]; then
  cat << EOF >> ${smv_scriptname}
  $viewpointd

EOF
fi
if [ "$viewpoint" != "" ]; then
  cat << EOF >> ${smv_scriptname}
SETVIEWPOINT
  $viewpoint
EOF
fi
  cat << EOF >> ${smv_scriptname}
LOADSLICERENDER
EOF
  slice_quantity=`cat $slicefilemenu | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2} }'`
  cat $slicefilemenu | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4} }' >> $smv_scriptname
  cat << EOF >> $smv_scriptname
  $img_basename 
  0 1
EOF
  echo ""
  cat << EOF > $img_scriptname
#!/bin/bash
NPROCS=$NPROCS
QUEUE=$QUEUE
SMOKEVIEW=$SMOKEVIEW
QSMV="$FIREMODELS/smv/Utilities/Scripts/qsmv.sh $SHARE $O_opt $v_opt"
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
slice_index=
HELP_ALL=
JOBPREFIX=SV_
GENERATE_IMAGES=
MAKE_MOVIE=

CONFIGDIR=$HOME/.smokeview
if [ ! -e $CONFIGDIR ]; then
  mkdir $CONFIGDIR
fi
GLOBALCONFIG=$CONFIGDIR/slice2mp4_global

SMVSCRIPTDIR=
touch test.$$ >& /dev/null
if [ -e test.$$ ]; then
  rm test.$$
else
  SMVSCRIPTDIR=${CONFIGDIR}/
fi


# define repo variables

CURDIR=`pwd`
SCRIPTDIR=`dirname "$0"`
cd $SCRIPTDIR/../../..
ROOTDIR=`pwd`
SMVREPO=$ROOTDIR/smv
cd $CURDIR
SMOKEVIEW=$SMVREPO/Build/smokeview/intel_linux_64/smokeview_linux_64
if [ ! -e $SMOKEVIEW ]; then
  SMOKEVIEW=$SMVREPO/Build/smokeview/intel_linux_64/smokeview_linux_test_64
fi
QSMV=$SMVREPO/Utilities/Scripts/qsmv.sh
MAKEMOVIE=$SMVREPO/Utilities/Scripts/make_movie.sh
EMAIL=
SHARE=
v_opt=
O_opt=

#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'e:hiOv' OPTION
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
  O)
   O_opt="-O"
   ;;
  v)
   v_opt="-v"
   ;;
esac
done
shift $(($OPTIND-1))

if [ ! -e $SMOKEVIEW ]; then
  echo "***error: smokeview not found at $SMOKEVIEW"
  exit 1
fi

input=$1
restore_state

smvfile=$1.smv
slicefilemenu=$CONFIGDIR/$1.slcf

if [ ! -e $smvfile ]; then
  echo "***error: $smvfile does not exist"
  exit
fi

$SMOKEVIEW -info $input >& /dev/null

# get viewpoint menu (optional)

nviewpoints=0
viewpointmenu=$CONFIGDIR/$1.viewpoints
if [ -e $viewpointmenu ]; then
  nviewpoints=`cat $viewpointmenu | wc -l`
  (( nviewpoints -= 3 ))
else
  echo "index   viewpoint"  > $viewpointmenu
  echo "d   delete"        >> $viewpointmenu
fi
echo "    x   VIEWXMIN"    >> $viewpointmenu
echo "    X   VIEWXMAX"    >> $viewpointmenu
echo "    y   VIEWYMIN"    >> $viewpointmenu
echo "    Y   VIEWYMAX (not working)"    >> $viewpointmenu
echo "    z   VIEWZMIN"    >> $viewpointmenu
echo "    Z   VIEWZMAX"    >> $viewpointmenu


# get slice file menu (required)

if [ ! -e $slicefilemenu ]; then
  echo "*** error: $slicefilemenu does not exist"
  exit
fi

nslices=`cat $slicefilemenu | wc -l`
(( nslices -= 2 ))
if [ $nslices  -eq 0 ]; then
  echo "*** error:  No slice files were found in $smvfile"
  exit
fi

select_slice_file

select_options

save_state


