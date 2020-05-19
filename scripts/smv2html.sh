#!/bin/bash
input=$1


slcffile=$1.slcf

OUTPUT_SLICES ()
{
  cat $slcffile | awk -F"," '{ print $1": ",$2," ",$3," ",$4}'
  nslices=`cat $slcffile | wc -l`
}

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
  echo "ans=$ans"
  if [[ "$ans" -ge 1 ]] && [[ "$ans" -le "$nslices" ]]; then
    echo converting index $ans
  else
    echo index $ans out of bounds
  fi
done

