#!/bin/bash

MAKEPO=../../makepo/intel_linux/makepo_linux
#cat *.c *.cpp | $MAKEPO | sort -u | $MAKEPO -a > smokeview_template.po
cd ../smokeview
cat *.c *.cpp | $MAKEPO | sort -u  > smokeview_template.po
