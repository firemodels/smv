#!/bin/bash
cd ../smokeview
cat *.c *.cpp *.h ../shared/*.h ../shared/*.c  | wc -l
