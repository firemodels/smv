$commit = "06449f1db6f42c41f0d43d71f1c46513f98c7d3c"

$zip_path = "$env:temp\test-data.zip"
$unzip_path = "$env:temp\test-data-out"
Invoke-WebRequest -Uri https://github.com/JakeOShannessy/fig/archive/$commit.zip -OutFile $zip_path
Expand-Archive $zip_path -DestinationPath $unzip_path -Force
mkdir fig -Force
Move-Item $unzip_path/fig-$commit/* fig -Force

