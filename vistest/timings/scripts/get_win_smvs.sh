#!/bin/bash
CURDIR=`pwd`
cd
if [ ! -d SMVS/win ]; then
  mkdir -p SMVS/win
fi
cd SMVS/win
wget -q https://github.com/firemodels/smv/releases/download/SMV6.7.1/SMV6.7.1_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.7.0/SMV6.7.0_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.6.5/SMV6.6.5_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.6.4/SMV6.6.4_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.6.3/SMV6.6.3_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.6.2/smv_6.6.2_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.6.1/smv_6.6.1_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.6.0/smv_6.6.0_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.5.5/smv_6.5.5_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.5.4/smv_6.5.4_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.5.3/smv_6.5.3_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.5.2/smv_6.5.2_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.5.1/smv_6.5.1_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.5.0/smv_6.5.0_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.4.4/smv_6.4.4_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.4.3/smv_6.4.3_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.4.2/smv_6.4.2_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.4.1/smv_6.4.1_win64.exe
wget -q https://github.com/firemodels/smv/releases/download/SMV6.4.0/smv_6.4.0_win64.exe
cd $CURDIR
