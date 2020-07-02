#!/bin/bash

directory=$1

cd ~/$directory
git describe --dirty 
