COMMIT=06449f1db6f42c41f0d43d71f1c46513f98c7d3c

TEMP_DIR=$(mktemp -d)
ZIP_PATH=$TEMP_DIR/test-data.zip
wget https://github.com/JakeOShannessy/fig/archive/$COMMIT.zip -O $ZIP_PATH
unzip $ZIP_PATH
mkdir -p fig
mv fig-$COMMIT/* fig
