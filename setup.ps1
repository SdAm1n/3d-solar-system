# Solar System 3D - Setup Script
# This script helps you set up the project dependencies using vcpkg

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Solar System 3D - Setup Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if vcpkg is installed
$vcpkgPath = "C:\vcpkg"
$customVcpkgPath = Read-Host "Enter vcpkg path (press Enter for default: C:\vcpkg)"

if ($customVcpkgPath -ne "") {
    $vcpkgPath = $customVcpkgPath
}

if (-not (Test-Path "$vcpkgPath\vcpkg.exe")) {
    Write-Host "vcpkg not found at $vcpkgPath" -ForegroundColor Red
    Write-Host ""
    Write-Host "Would you like to clone and install vcpkg now? (Y/N)" -ForegroundColor Yellow
    $install = Read-Host
    
    if ($install -eq "Y" -or $install -eq "y") {
        Write-Host "Cloning vcpkg..." -ForegroundColor Green
        
        # Create parent directory if it doesn't exist
        $parentDir = Split-Path $vcpkgPath -Parent
        if (-not (Test-Path $parentDir)) {
            New-Item -ItemType Directory -Path $parentDir -Force | Out-Null
        }
        
        git clone https://github.com/Microsoft/vcpkg.git $vcpkgPath
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "Bootstrapping vcpkg..." -ForegroundColor Green
            & "$vcpkgPath\bootstrap-vcpkg.bat"
            
            Write-Host "Integrating vcpkg with Visual Studio..." -ForegroundColor Green
            & "$vcpkgPath\vcpkg.exe" integrate install
        } else {
            Write-Host "Failed to clone vcpkg. Please install it manually." -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "Please install vcpkg manually and run this script again." -ForegroundColor Yellow
        Write-Host "Visit: https://github.com/Microsoft/vcpkg" -ForegroundColor Cyan
        exit 1
    }
}

Write-Host ""
Write-Host "Installing required packages..." -ForegroundColor Green
Write-Host ""

# Install packages
$packages = @("glad:x64-windows", "glfw3:x64-windows", "glm:x64-windows")

foreach ($package in $packages) {
    Write-Host "Installing $package..." -ForegroundColor Cyan
    & "$vcpkgPath\vcpkg.exe" install $package
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "? $package installed successfully" -ForegroundColor Green
    } else {
        Write-Host "? Failed to install $package" -ForegroundColor Red
    }
    Write-Host ""
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Setup Complete!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Open the solution in Visual Studio" -ForegroundColor White
Write-Host "2. Set the configuration to 'Release' or 'Debug'" -ForegroundColor White
Write-Host "3. Set the platform to 'x64'" -ForegroundColor White
Write-Host "4. Build and run the project (F5)" -ForegroundColor White
Write-Host ""
Write-Host "If you encounter any issues, check the README.md file." -ForegroundColor Cyan
Write-Host ""

Read-Host "Press Enter to exit"
