#!/bin/bash
firesize=$1
doorheight=$2
gravx=$3
gravz=$4
CHID=simple1
outfile=${CHID}.fds
cat << EOF > $outfile
&HEAD CHID='$CHID', TITLE='Simple demonstration case.'  /   
&MESH IJK=36,24,24, XB=0.0,3.6,0.0,2.4,0.0,2.4 /             
&MISC GVEC=$gravx,0.0,$gravz /                                     
&TIME T_END=60.0 /                                           
&REAC FUEL='PROPANE', SOOT_YIELD=0.01 /                        
&SURF ID='BURNER', HRRPUA=$firesize, COLOR='RED' /          
&OBST XB=0.0,0.4,1.0,1.4,0.0,0.2 /                           
&VENT XB=0.0,0.4,1.0,1.4,0.2,0.2, SURF_ID='BURNER' /         
&VENT XB=3.6,3.6,0.8,1.6,0.0,$doorheight, SURF_ID='OPEN' /  
&BNDF QUANTITY='GAUGE HEAT FLUX' /                           
&SLCF PBY=1.2, QUANTITY='TEMPERATURE', VECTOR=.TRUE. /       
&TAIL /                                                      
EOF
