#!/bin/bash
CONVERT=../../Build/convert/intel_osx_64/convert_osx_64

$CONVERT < glut_hel20.c.base > glut_hel20.c
$CONVERT < glut_hel24.c.base > glut_hel24.c
$CONVERT < glut_hel36.c.base > glut_hel36.c
