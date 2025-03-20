#!/bin/bash
cd ../smokeview
cat *.c *.cpp *.h ../shared/*.c ../shared/*.h ../background/*.c ../background/*.h ../smokediff/*.h ../smokediff/*.c ../smokezip/*.c ../smokezip/*.h ../wind2fds/*.c ../wind2fds/*.h| wc -l
