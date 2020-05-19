#!/bin/bash
input=$1


smvfile=$1.smv
slcffile=$1.slcf

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


OUTPUT_SLICES ()
{
  cat $slcffile | awk -F"," '{ print $1": ",$2," ",$3," ",$4}'
  nslices=`cat $slcffile | wc -l`
}

GENERATE_SCRIPT ()
{
  ind=$1
  scriptname=$2
  scriptname=${input}_slice_${ind}.ssf
  htmlbase=${input}_slice_${ind}
  cat << EOF > $scriptname
RENDERHTMLDIR
  .
UNLOADALL
LOADSLICE
EOF
  cat $slcffile | awk -v ind="$ind" -F"," '{ if($1 == ind){print $2"\n" $3 $4"\n"} }' >> $scriptname
  cat << EOF >> $scriptname
RENDERHTMLALL
  $htmlbase
EOF
}

is_file_installed smokeview || exit 1

if [ ! -e $smvfile ]; then
  echo "***error: $smvfile does not exist"
fi

if [ ! -e $slcffile ]; then
  echo "*** The file $slcffile does not exist (list of available slice files)"
  echo "Creating $slcffile"
  smokeview -slice_info $input >& /dev/null
fi

if [ ! -e $slcffile ]; then
  echo "*** error: $slicffile does not exist"
  exit 1
fi

while true; do
  OUTPUT_SLICES
  echo "x - exit script"
  read -p "Select slice index to generate an html file: " ans
  if [ "$ans" == "x" ]; then
    exit 
  fi
  if [ "$ans" == "r" ]; then
    echo continue 
  fi
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    echo converting index $ans
    scriptname=${input}_slice_${ans}.ssf
    rm -f $scriptname
    GENERATE_SCRIPT $ans $scriptname
    if [ -e $scriptname ]; then
      echo "***ceating html file"
      smokeview -htmlscript $scriptname $input
    fi
  else
    echo index $ans out of bounds
  fi
done

