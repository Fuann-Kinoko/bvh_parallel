param (
    [string]$action = "build"
)

$buildDir = "build"
$extraFlags = ""

function Make-Build {
    param (
        [string]$sourceFile,
        [string]$sourceFileDebug,
        [string]$destinationFile,
        [string]$destinationFileDebug
    )

    if (-not (Test-Path $sourceFile)) {
        return $false
    }

    cmake --build $buildDir $extraFlags

    $build_result = $false;
    if ($LASTEXITCODE -eq 0) {
        $build_result = $true;
    }

    $sourceLastWrite = (Get-Item $sourceFile).LastWriteTime

    if (Test-Path $destinationFile) {
        $destLastWrite = (Get-Item $destinationFile).LastWriteTime
        if ($sourceLastWrite -gt $destLastWrite) {
            Copy-Item -Path $sourceFile -Destination $destinationFile -Force
            Copy-Item -Path $sourceFileDebug -Destination $destinationFileDebug -Force
            return $build_result
        } else {
            return $build_result
        }
    } else {
        Copy-Item -Path $sourceFile -Destination $destinationFile -Force
        Copy-Item -Path $sourceFileDebug -Destination $destinationFileDebug -Force
        return $build_result
    }
}

# Push-Location -Path $buildDir
try {
    $source = "build/Debug/BVHVisualization.exe"
    $sourceDBG = "build/Debug/BVHVisualization.pdb"
    $dest = "./BVHVisualization.exe"
    $destDBG = "./BVHVisualization.pdb"

    if ($action -eq "build") {
        Make-Build -sourceFile $source -destinationFile $dest -sourceFileDebug $sourceDBG -destinationFileDebug $destDBG
    }
    if ($action -eq "run") {
        $build_result = Make-Build -sourceFile $source -destinationFile $dest -sourceFileDebug $sourceDBG -destinationFileDebug $destDBG
        if ($build_result -eq $true) {
            ./BVHVisualization.exe $args
        }
    }
} finally {
   # Pop-Location
}
