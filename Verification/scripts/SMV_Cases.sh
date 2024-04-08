#!/bin/bash

# add -A to any case that you wish to be a part of the benchmark timing suite

$QFDS -d Visualization -t -A mplume5c8_bench.fds
$QFDS -d Visualization -t -A plume5c_bench.fds
$QFDS -p 2 -d Visualization -t -A thouse5_bench.fds

$QFDS -d Visualization boundtest.fds
$QFDS -d Visualization cell_test.fds
$RUNCFAST -d Visualization cfast_test.in
$QFDS -d Visualization colorbar.fds
$QFDS -d Visualization colorbar2.fds
$QFDS -d Visualization colorconv.fds
$QFDS -d Visualization hvac_comp.fds
$QFDS -d Visualization -p 8 -n 8 mplume5c8.fds
$QFDS -d Visualization objects_dynamic.fds
$QFDS -d Visualization objects_elem.fds
$QFDS -d Visualization objects_static.fds
$QFDS -d Visualization part_color.fds
$QFDS -d Visualization plume5c.fds
$QFDS -d Visualization plume5cdelta.fds
$QFDS -d Visualization plumeiso.fds
$QFDS -d Visualization plume_average.fds
$QFDS -p 8 -d Visualization plumeiso8.fds
$QFDS -d Visualization plume5c_bounddef.fds
$QFDS -d Visualization script_test.fds
$QFDS -d Visualization script_slice_test.fds
$QFDS -d Visualization sillytexture.fds
$QFDS -d Visualization slicemask.fds
$QFDS -d Visualization smoke_sensor.fds
$QFDS -d Visualization smoke_test.fds
$QFDS -d Visualization smoke_test2.fds
$QFDS -d Visualization smoke_test3.fds
$QFDS -d Visualization smoke1.fds
$QFDS -d Visualization smoke2.fds
$QFDS -d Visualization smokex010.fds
$QFDS -d Visualization smokex020.fds
$QFDS -d Visualization smokex040.fds
$QFDS -d Visualization smokex080.fds
$QFDS -d Visualization smokex160.fds
$QFDS -d Visualization smoke_test_geom.fds
$QFDS -d Visualization sprinkler_many.fds
$QFDS -p 6 -d Visualization sphere_propaneu.fds
$QFDS -p 6 -d Visualization sphere_propanec.fds
$QFDS -d Visualization testsort.fds
$QFDS -p 2 -d Visualization thouse5.fds
$QFDS -p 2 -d Visualization thouse5delta.fds
$QFDS -d Visualization tour.fds
$QFDS -d Visualization transparency.fds
$QFDS -d Visualization vcirctest.fds
$QFDS -d Visualization vcirctest2.fds
$QFDS -p 3 -d Visualization vectorskipx.fds
$QFDS -p 3 -d Visualization vectorskipy.fds
$QFDS -p 3 -d Visualization vectorskipz.fds
$QFDS -d Visualization version.fds
$QFDS -d Visualization version2.fds
$QFDS -d Visualization windrose.fds
