#!/bin/sh
COMMIT=97c852beff8c6142ec09dc0092381a605d08d943

TEMP_DIR=$(mktemp -d)
ZIP_PATH=$TEMP_DIR/test-data.zip
wget --tries=5 https://github.com/firemodels/fig/archive/$COMMIT.zip -O "$ZIP_PATH"
unzip "$ZIP_PATH"
mkdir -p fig
mv fig-$COMMIT/* fig
