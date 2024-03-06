$commit = "83e6a770f511e315af21be5868d758b99f0a83ad"

$zip_path = "$env:temp\test-data.zip"
$unzip_path = "$env:temp\test-data-out"
Invoke-WebRequest -Uri https://github.com/firemodels/fig/archive/$commit.zip -OutFile $zip_path
Expand-Archive $zip_path -DestinationPath $unzip_path -Force
mkdir fig -Force
Move-Item $unzip_path/fig-$commit/* fig -Force
Copy-Item -Force -Recurse fig/smv/Tests/Visualization ../Verification
