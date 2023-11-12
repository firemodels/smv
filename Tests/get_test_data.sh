#!/bin/sh
COMMIT=bf2446b600b4062196923a74cd2cf67c3b7be76d

TEMP_DIR=$(mktemp -d)
ZIP_PATH=$TEMP_DIR/test-data.zip
wget --tries=5 https://github.com/firemodels/fig/archive/$COMMIT.zip -O "$ZIP_PATH"
unzip "$ZIP_PATH"
rm -rf fig
mkdir -p fig
mv -f fig-$COMMIT/* fig
cp -rf fig/smv/Tests/Visualization ../Verification
