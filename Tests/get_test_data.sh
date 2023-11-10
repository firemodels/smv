#!/bin/sh
COMMIT=67300634bc63eb9a6959abf6623b4922ff7b1e34

TEMP_DIR=$(mktemp -d)
ZIP_PATH=$TEMP_DIR/test-data.zip
wget --tries=5 https://github.com/JakeOShannessy/fig/archive/$COMMIT.zip -O "$ZIP_PATH"
unzip "$ZIP_PATH"
rm -rf fig
mkdir -p fig
mv -f fig-$COMMIT/* fig
cp -rf fig/smv/Tests/Visualization ../Verification
