#!/bin/bash
$QFDS -d Visualization boundtest.fds
$QFDS -d Visualization cad_test.fds
$QFDS -d Visualization cell_test.fds
$RUNCFAST -d Visualization cfast_test.in
$QFDS -d Visualization colorbar.fds
$QFDS -d Visualization colorbar2.fds
$QFDS -d Visualization colorconv.fds
$QFDS -d Visualization color_geom.fds
$QFDS -d Visualization fed_test.fds
$QFDS -d Visualization hvac_comp.fds
$QFDS -d Visualization -p 8 -n 8 mplume5c8.fds
$QFDS -d Visualization objects_dynamic.fds
$QFDS -d Visualization objects_elem.fds
$QFDS -d Visualization objects_static.fds
$QFDS -p 2 -d Visualization obst_remove.fds
$QFDS -d Visualization obst_test1.fds
$QFDS -p 4 -d Visualization obst_test4.fds
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
$QFDS -d Visualization -p 6 sphere_propaneu.fds
$QFDS -d Visualization -p 6 sphere_propanec.fds
$QFDS -d Visualization testsort.fds
$QFDS -d Visualization -p 4 thouse5.fds
$QFDS -d Visualization -p 4 thouse5delta.fds
$QFDS -d Visualization tour.fds
$QFDS -d Visualization transparency.fds
$QFDS -d Visualization vcirctest.fds
$QFDS -d Visualization -p 3 vectorskipx.fds
$QFDS -d Visualization -p 3 vectorskipy.fds
$QFDS -d Visualization -p 3 vectorskipz.fds
$QFDS -d Visualization version.fds
$QFDS -d Visualization version2.fds
$QFDS -d Visualization windrose.fds
