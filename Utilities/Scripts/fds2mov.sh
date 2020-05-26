#!/bin/bash

#---------------------------------------------
#                   Usge
#---------------------------------------------

function Usage {
  scriptname=`basename $0`
  echo "Usage: $scriptname [options] casename"
  echo ""
  echo "This script generates a smkeview script used to generate images for creating an animation"
  echo ""
  exit
}


#---------------------------------------------
#                   is_file_installed
#---------------------------------------------

is_file_installed()
{
  local program=$1

  out=/tmp/program.out.$$
  $program -v >& $out
  notfound=`cat $out | tail -1 | grep "not found" | wc -l`
  rm $out
  if [ "$notfound" == "1" ] ; then
    echo "***error: $program not installed" 
    return 1
  fi
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
  echo "smokeview script $scriptname generated"
  cat << EOF > $makemovie
#!/bin/bash
NPROC=1
PROCPERNODE=1
QUEUE=batch
EXE=`which smokeview`
qsmv.sh -p \$NPROC -n \$PROCPERNODE -q \$QUEUE -e \$EXE -c $scriptname $input
EOF
}

#*** initialize variables

RENDERDIR=.

#---------------------------------------------
#                  parse command line options 
#---------------------------------------------

while getopts 'a:hr:' OPTION
do
case $OPTION  in
  r)
   RENDERDIR="$OPTARG"
   ;;
  h)
   Usage
   exit
   ;;
esac
done
shift $(($OPTIND-1))

input=$1

smvfile=$1.smv
slcffile=$1.slcf

is_file_installed smokeview || exit 1

if [ ! -e $smvfile ]; then
  echo "***error: $smvfile does not exist"
  exit
fi

if [ ! -e $slcffile ]; then
  smokeview -slice_info $input >& /dev/null
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
  echo "x - exit script"
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

