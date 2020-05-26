#!/bin/bash

#---------------------------------------------
#                   Usge
#---------------------------------------------

function Usage {
  scriptname=`basename $0`
  echo "Usage: $scriptname [options] casename"
  echo ""
  echo "-h - display this message"
  echo "-p - number of processes"
  echo "-q - queue"
  echo "-r - directory containing generated images. [default: $RENDERDIR]"
  if [ "$SMOKEVIEW" == "" ]; then
    echo "-s - full path of smokeview executable."
  else
    echo "-s - full path of smokeview executable. [default: $SMOKEVIEW]"
  fi
  echo ""
  exit
}


#---------------------------------------------
#                   is_file_installed
#---------------------------------------------

get_smokeview ()
{
  if [ "$SMOKEVIEW" != "" ]; then
    if [ -e $SMOKEVIEW ]; then
      return 0
    fi
    echo "***error: The smokeview file, $SMOKEVIEW, does not exist. Add smokeview to"
    echo "your path or define the environment variable SMOKEVIEW in a startup file"
    echo "that points to smokeview's location."
    return 1
  fi
  out=/tmp/program.out.$$
  smokeview -v >& $out
  notfound=`cat $out | tail -1 | grep "not found" | wc -l`
  rm $out
  if [ "$notfound" == "1" ] ; then
    echo "***error: smokeview is not installed. Add smokeview to your PATH or" 
    echo "define the environment variable SMOKEVIEW in a startup file that points "
    echo "to smokeview's location."
    return 1
  fi
  SMOKEVIEW=`which smokeview`
  return 0
}

#---------------------------------------------
#                   OUTPUT_SLICES
#---------------------------------------------

OUTPUT_SLICES ()
{
  cat $slcffile | awk -F"," '{ print $1" ",$2," ",$3," ",$4}'
}

#---------------------------------------------
#                   GENERATE_SCRIPT
#---------------------------------------------

GENERATE_SCRIPT ()
{
  ind=$1
  scriptname=$2
  basename=${input}_slice_${ind}
  scriptname=${basename}.ssf
  makemovie=${basename}.sh
  cat << EOF > $scriptname
RENDERDIR
  $RENDERDIR
UNLOADALL
LOADSLICERENDER
EOF
  cat $slcffile | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4} }' >> $scriptname
  cat << EOF >> $scriptname
  $basename 
  0 1
EOF
  echo "smokeview script named $scriptname generated"
  cat << EOF > $makemovie
#!/bin/bash
qsmv.sh -P $NPROC -q $QUEUE -e $SMOKEVIEW -c $scriptname $input
EOF
}

#*** initialize variables

RENDERDIR=.
NPROC=1
QUEUE=batch

#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'hp:q:r:s:' OPTION
do
case $OPTION  in
  h)
   Usage
   exit
   ;;
  p)
   NPROC="$OPTARG"
   ;;
  q)
   QUEUE="$OPTARG"
   ;;
  r)
   RENDERDIR="$OPTARG"
   ;;
  s)
   SMOKEVIEW="$OPTARG"
   ;;
esac
done
shift $(($OPTIND-1))

input=$1

smvfile=$1.smv
slcffile=$1.slcf

get_smokeview || exit 1

if [ ! -e $smvfile ]; then
  echo "***error: $smvfile does not exist"
  exit
fi

if [ ! -e $slcffile ]; then
  $SMOKEVIEW -slice_info $input >& /dev/null
fi

if [ ! -e $slcffile ]; then
  echo "*** error: $slcffile does not exist"
  exit
fi

nslices=`cat $slcffile | wc -l`
if [ "$nslices" == "0" ]; then
  echo "*** error:  No slice files found in $smvfile"
  exit
fi

while true; do
  OUTPUT_SLICES
  echo "x - make smokeview script $scriptname"
  read -p "Select slice index: " ans
  if [ "$ans" == "x" ]; then
    exit 
  fi
  if [ "$ans" == "r" ]; then
    echo continue 
  fi
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    echo creating smokeview script using $ans
    scriptname=${input}_slice_${ans}.ssf
    rm -f $scriptname
    GENERATE_SCRIPT $ans $scriptname
  else
    echo index $ans out of bounds
  fi
done

