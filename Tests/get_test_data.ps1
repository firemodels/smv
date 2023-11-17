$commit = "bf2446b600b4062196923a74cd2cf67c3b7be76d"

$zip_path = "$env:temp\test-data.zip"
$unzip_path = "$env:temp\test-data-out"
Invoke-WebRequest -Uri https://github.com/firemodels/fig/archive/$commit.zip -OutFile $zip_path
Expand-Archive $zip_path -DestinationPath $unzip_path -Force
mkdir fig -Force
Move-Item $unzip_path/fig-$commit/* fig -Force
Copy-Item -Force -Recurse fig/smv/Tests/Visualization ../Verification
