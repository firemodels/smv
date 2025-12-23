@echo off
if exist gd.out      grep -i warning: gd.out      > gd.wrn
if exist glui.out    grep -i warning: glui.out    > glui.wrn
if exist glut32.out  grep -i warning: glut32.out  > glut32.wrn
if exist jpeg.out    grep -i warning: jpeg.out    > jpeg.wrn
if exist png.out     grep -i warning: png.out     > png.wrn
if exist pthread.out grep -i warning: pthread.out > pthread.wrn
if exist zlib.out    grep -i warning: zlib.out    > zlib.wrn