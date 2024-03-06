#!/bin/sh
COMMIT=83e6a770f511e315af21be5868d758b99f0a83ad

TEMP_DIR=$(mktemp -d)
ZIP_PATH=$TEMP_DIR/test-data.zip
wget --tries=5 https://github.com/firemodels/fig/archive/$COMMIT.zip -O "$ZIP_PATH"
unzip "$ZIP_PATH"
rm -rf fig
mkdir -p fig
mv -f fig-$COMMIT/* fig
cp -rf fig/smv/Tests/Visualization ../Verification
