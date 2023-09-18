#!/bin/bash
DOWNLOADFIGURES ()
{
  FILE=$1_figures
  echo ***Downloading $FILE.tar.gz
  gh release download SMOKEVIEW_TEST -p $FILE.tar.gz -R github.com/firemodels/test_bundles --clobber
  if [ ! -e $FILE.tar.gz ]; then
    echo ***error: $FILE.tar.gz failed to download
    echo command: gh release download FDS_TEST -p $FILE.tar.gz -R github.com/firemodels/test_bundles --clobber
  else
    echo "   successful"
  fi
}
DOWNLOADFIGURES SMV_UG
DOWNLOADFIGURES SMV_VG

SCRIPTDIR=`pwd`
cd ..
MANDIR=`pwd`

cd $SCRIPTDIR
rm -rf FIGS
mkdir FIGS
mkdir FIGS/SMV_UG
mkdir FIGS/SMV_VG

SBUG=$SCRIPTDIR/FIGS/SMV_UG
SBVG=$SCRIPTDIR/FIGS/SMV_VG

# Copy User's Guide Figures
if [ -e $SCRIPTDIR/SMV_UG_figures.tar.gz ]; then
  cd $SBUG
  tar xf $SCRIPTDIR/SMV_UG_figures.tar.gz
  cp * $MANDIR/SMV_User_Guide/SCRIPT_FIGURES/
  echo Smokeview Users Guide figures copied to SMV_User_Guide/SCRIPT_FIGURES
fi

# Copy Verification Guide Figures
if [ -e $SCRIPTDIR/SMV_VG_figures.tar.gz ]; then
  cd $SBVG
  tar xf $SCRIPTDIR/SMV_VG_figures.tar.gz
  cp * $MANDIR/SMV_Verification_Guide/SCRIPT_FIGURES/.
  echo SMV Verification Guide figures copied to SMV_Verification_Guide_Guide/SCRIPT_FIGURES
fi
