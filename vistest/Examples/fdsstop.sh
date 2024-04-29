#!/bin/bash
chid=$1
touch ${chid}.stop
rm -f ${chid}.running
