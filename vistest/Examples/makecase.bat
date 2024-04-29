@echo off
set firesize=%1
set doorheight=%2
set gravx=%3
set gravy=%4
set gravz=%5
set CHID=%6
set outfile=%CHID%.fds
echo ^&HEAD CHID='%CHID%', TITLE='Simple demonstration case.'  /    > %outfile%
echo ^&MESH IJK=48,24,24, XB=0.0,4.8,0.0,2.4,0.0,2.4 /             >> %outfile%
echo ^&MISC GVEC=%gravx%,%gravy%,%gravz% /                         >> %outfile%
echo ^&TIME T_END=60.0 /                                           >> %outfile%
echo ^&REAC FUEL='PROPANE', SOOT_YIELD=0.01 /                      >> %outfile%  
echo ^&SURF ID='BURNER', HRRPUA=%firesize%, COLOR='RED' /          >> %outfile%
echo ^&OBST XB=0.0,0.4,1.0,1.4,0.0,0.2 /                           >> %outfile%
echo ^&VENT XB=0.0,0.4,1.0,1.4,0.2,0.2, SURF_ID='BURNER' /         >> %outfile%
echo ^&OBST XB=3.5,3.6,0.0,2.4,0.0,2.4 /                           >> %outfile%
echo ^&HOLE XB=3.4,3.7,0.8,1.6,0.0,%doorheight% /                  >> %outfile%
echo ^&VENT XB=3.6,4.8,0.0,2.4,2.4,2.4, SURF_ID='OPEN' /           >> %outfile%
echo ^&VENT XB=3.6,4.8,0.0,0.0,0.0,2.4, SURF_ID='OPEN' /           >> %outfile%
echo ^&VENT XB=3.6,4.8,2.4,2.4,0.0,2.4, SURF_ID='OPEN' /           >> %outfile%
echo ^&VENT XB=4.8,4.8,0.0,2.4,0.0,2.4, SURF_ID='OPEN' /           >> %outfile%
echo ^&BNDF QUANTITY='GAUGE HEAT FLUX' /                           >> %outfile%
echo ^&SLCF PBY=1.2, QUANTITY='TEMPERATURE', VECTOR=.TRUE. /       >> %outfile%
echo ^&TAIL /                                                      >> %outfile%


