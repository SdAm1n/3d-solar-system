# Copy Shaders Script
# This script copies all shader files to the output directory

param(
    [string]$TargetDir,
    [string]$ProjectDir
)

# All shader files in the shaders folder
$shaderFiles = @(
    "shaders/planet.vs", "shaders/planet.fs",
    "shaders/orbit.vs", "shaders/orbit.fs",
    "shaders/skybox.vs", "shaders/skybox.fs"
)

Write-Host "Copying shader files to output directory..." -ForegroundColor Cyan
Write-Host "Target: $TargetDir" -ForegroundColor Gray

# Create shaders directory in target if it doesn't exist
$targetShadersDir = Join-Path $TargetDir "shaders"
if (-not (Test-Path $targetShadersDir)) {
    New-Item -ItemType Directory -Path $targetShadersDir -Force | Out-Null
    Write-Host "  Created shaders directory in output" -ForegroundColor Cyan
}

$successCount = 0
$failCount = 0

foreach ($shader in $shaderFiles) {
    $source = Join-Path $ProjectDir $shader
    $dest = Join-Path $TargetDir $shader
    
    if (Test-Path $source) {
        Copy-Item -Path $source -Destination $dest -Force
        Write-Host "  [OK] Copied: $shader" -ForegroundColor Green
        $successCount++
    } else {
        Write-Host "  [!!] Warning: $shader not found at $source" -ForegroundColor Yellow
        $failCount++
    }
}

Write-Host ""
Write-Host "Summary: $successCount copied, $failCount failed" -ForegroundColor Cyan
if ($failCount -eq 0) {
    Write-Host "All shader files copied successfully!" -ForegroundColor Green
}
