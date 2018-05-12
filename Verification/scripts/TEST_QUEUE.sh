#!/bin/bash
numcases=$1

QUEUE=single
fdsexe=../../../fds/Build/impi_intel_linux_64/fds_impi_intel_linux_64
qfds=../../../fds/Utilities/Scripts/qfds.sh

echo numcases=$numcases
if [ $numcases -gt 999 ]; then
  numcases=999
fi

MAKE_CASE ()
{
CASE=$1
cat << EOF > $CASE.fds
&HEAD CHID='$CASE',TITLE='Plume whirl case' /

  same as plume5a except there is a blockage in the middle of the scene to block the flow
  The purpose of this case is to demonstrate the curved flow (via streak lines) that results.

&MESH IJK=16,16,32, XB=0.0,1.6,0.0,1.6,0.0,3.2 /

&MISC IBLANK_SMV=.TRUE. /

&DUMP NFRAMES=400 DT_PL3D=8.0, DT_SL3D=0.1 /

&INIT XB=0.2,1.4,0.2,1.4,0.5,2.2  TEMPERATURE=600.0 /

&TIME T_END=40. /  Total simulation time

&MATL ID                    = 'FABRIC'
      FYI                   = 'Properties completely fabricated'
      SPECIFIC_HEAT         = 1.0
      CONDUCTIVITY          = 0.1
      DENSITY               = 100.0
      N_REACTIONS           = 1
      NU_SPEC               = 1.
      SPEC_ID               = 'PROPANE'
      REFERENCE_TEMPERATURE = 350.
      HEAT_OF_REACTION      = 3000.
      HEAT_OF_COMBUSTION    = 15000. /

&MATL ID                    = 'FOAM'
      FYI                   = 'Properties completely fabricated'
      SPECIFIC_HEAT         = 1.0
      CONDUCTIVITY          = 0.05
      DENSITY               = 40.0
      N_REACTIONS           = 1
      NU_SPEC               = 1.
      SPEC_ID               = 'PROPANE'
      REFERENCE_TEMPERATURE = 350.
      HEAT_OF_REACTION      = 1500.
      HEAT_OF_COMBUSTION    = 30000. /

&SURF ID             = 'UPHOLSTERY_LOWER'
      FYI            = 'Properties completely fabricated'
      RGB            = 151,96,88
      BURN_AWAY      = .FALSE.
      MATL_ID(1:2,1) = 'FABRIC','FOAM'
      THICKNESS(1:2) = 0.002,0.1 
/

&SURF ID             = 'UPHOLSTERY_UPPER'
      FYI            = 'Properties completely fabricated'
      RGB            = 151,96,88
      BURN_AWAY      = .FALSE.
      TMP_FRONT      = 600.0
/
&REAC SOOT_YIELD=0.01,FUEL='PROPANE'/
&SURF ID='BURNER',HRRPUA=600.0,PART_ID='tracers' /  Ignition source

&VENT XB=0.5,1.1,0.5,1.1,0.1,0.1,SURF_ID='BURNER' /  fire source on kitchen stove

&OBST XB=0.5,1.1,0.5,1.1,0.0,0.1 /
&OBST XB=0.3,1.3,0.3,1.3,0.4,0.8 SURF_ID='UPHOLSTERY_LOWER'/
&HOLE XB=0.6,1.0,0.2,0.8,0.3,0.9 /
&OBST XB=0.3,1.3,0.3,1.3,1.2,1.6 SURF_ID='UPHOLSTERY_UPPER' /

&VENT XB=0.0,1.6,0.0,0.0,0.0,3.2,SURF_ID='OPEN'/
&VENT XB=1.6,1.6,0.0,1.6,0.0,3.2,SURF_ID='OPEN'/
&VENT XB=0.0,1.6,1.6,1.6,0.0,3.2,SURF_ID='OPEN'/
&VENT XB=0.0,0.0,0.0,1.6,0.0,3.2,SURF_ID='OPEN'/
&VENT XB=0.0,1.6,0.0,1.6,3.2,3.2,SURF_ID='OPEN'/

&PART ID='tracers',MASSLESS=.TRUE.,
   QUANTITIES(1:3)='U-VELOCITY','V-VELOCITY','W-VELOCITY'
   SAMPLING_FACTOR=10 /  Description of massless tracer particles. Apply at a

&TAIL /
EOF
}

count=0  
while [ $count -le $numcases ]  
do  
    case=$count
    if [ $count -lt 10 ]; then
      case=00$count
    else
      if [ $count -lt 100 ]; then
        case=0$count
      fi
    fi
    MAKE_CASE test$case
    $qfds -e $fdsexe -q $QUEUE test$case.fds

    count=$(( $count + 1 ))
done  
