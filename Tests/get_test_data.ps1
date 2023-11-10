$commit = "67300634bc63eb9a6959abf6623b4922ff7b1e34"

$zip_path = "$env:temp\test-data.zip"
$unzip_path = "$env:temp\test-data-out"
Invoke-WebRequest -Uri https://github.com/JakeOShannessy/fig/archive/$commit.zip -OutFile $zip_path
Expand-Archive $zip_path -DestinationPath $unzip_path -Force
mkdir fig -Force
Move-Item $unzip_path/fig-$commit/* fig -Force
Copy-Item -Force -Recurse fig/smv/Tests/Visualization ../Verification
