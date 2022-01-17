#!/bin/bash
INFILE=$1
CHID=${INFILE%.*}

if [ $# -ne 1 ];  then
  cat << EOF

 This script creates an FDS input file using &BNDF and &SLCF quantities found
 in a table of frequently used output quanties contained in the FDS User Guide.

 Usage:  $0 casename.fds

EOF
exit
fi
CURDIR=`pwd`

cd ../../../fds/Manuals/FDS_User_Guide
FDSUGDIR=`pwd`
cd $CURDIR
FILE=$FDSUGDIR/FDS_User_Guide.tex
TEMP=/tmp/temp.$$

cat << EOF > $INFILE
&HEAD CHID='$CHID',TITLE='Generate Slice and Boundary File Labels' /

&MESH IJK=32,32,32, XB=0.0,1.6,0.0,1.0,0.0,1.6 /

&MISC IBLANK_SMV=.TRUE. /

&OBST XB=0.0,1.2,0.0,1.2,0.0,0.2 /
&VENT MB='XMIN', SURF_ID='OPEN' /
&VENT MB='XMAX', SURF_ID='OPEN' /
&VENT MB='YMIN', SURF_ID='OPEN' /
&VENT MB='YMAX', SURF_ID='OPEN' /
&VENT MB='ZMAX', SURF_ID='OPEN' /

&TIME T_END=1. /

&REAC SOOT_YIELD=0.01,FUEL='PROPANE'/
&SURF ID='BURNER',HRRPUA=10.0,PART_ID='tracers' /

&OBST XB=0.2,1.2,0.2,1.2,0.0,0.2 /
&VENT XB=0.2,1.2,0.2,1.2,0.2,0.2,SURF_ID='BURNER' /

&PART ID='tracers',MASSLESS=.TRUE.,
   QUANTITIES(1:3)='PARTICLE U','PARTICLE V','PARTICLE W'
   SAMPLING_FACTOR=10 /

EOF

cat $FILE  | sed -e '1,/caption\[Freq/d' | \
             sed -e '/end{longtable/,$d' | \
             sed -e '1,/endhead/d' | \
             awk -F'&' '{print $1 " & " $4}' | \
             sed 's/{\\ct //g' | \
             sed 's/ *^//g' | \
             sed 's/\\\\//g' | \
             sed 's/\\hline//g' |
             sed -r 's/}.*\&/   \&/g' |\
             sed 's/\\_/_/g' | sort >& $TEMP

cat $TEMP    | awk -F'&' '$2 ~ /.*B.*/ { print "&BNDF QUANTITY=\""$1"\" /" }' |\
               sed 's/ *"/"/g' >> $INFILE
echo >> $INFILE
cat $TEMP    | awk -F'&' '$2 ~ /.*S.*/ { print "&SLCF PBY=0.5, QUANTITY=\""$1"\" /" }' |\
               sed 's/ *"/"/g' >> $INFILE

cat << EOF >> $INFILE

&TAIL /
EOF
rm $TEMP
