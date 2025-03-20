#!/bin/bash
cd ../smokeview
grep '#ifdef' *.c *.cpp *.h ../shared/*.c ../shared/*.cpp ../shared/*.h | awk '{print $2}' | sort -u
