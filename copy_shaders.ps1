# Copy Shaders Script
# This script copies all shader files to the output directory

param(
    [string]$TargetDir,
    [string]$ProjectDir
)

# All shader files in the project
$shaderFiles = @(
    "planet.vs", "planet.fs",
    "orbit.vs", "orbit.fs",
    "skybox.vs", "skybox.fs"
)

Write-Host "Copying shader files to output directory..." -ForegroundColor Cyan
Write-Host "Target: $TargetDir" -ForegroundColor Gray

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
