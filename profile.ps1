$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

$BuildDir = ".\build\profile"
$TargetExe = ".\build\profile\bin\profile_app.exe"

Write-Host "==== Configure Profile Build ====" -ForegroundColor Cyan
cmake --preset profile

Write-Host "==== Build Profile Target ====" -ForegroundColor Cyan
cmake --build --preset profile

Write-Host "==== Check Target Exe ====" -ForegroundColor Cyan
if (-not (Test-Path $TargetExe)) {
    Write-Host "Target executable not found: $TargetExe" -ForegroundColor Red
    Write-Host "Available executables:" -ForegroundColor Yellow
    Get-ChildItem "$BuildDir\bin\*.exe" -ErrorAction SilentlyContinue
    exit 1
}

Write-Host "==== Run Samply ====" -ForegroundColor Cyan
samply record $TargetExe