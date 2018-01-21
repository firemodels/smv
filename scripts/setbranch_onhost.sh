#!/bin/bash

directory=$1
host=$2

ssh -q $host \( cd \~/$directory \; git checkout master  \)
