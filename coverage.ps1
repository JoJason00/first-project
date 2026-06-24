$ErrorActionPreference = "Stop"

# 切到脚本所在目录，也就是项目根目录
Set-Location $PSScriptRoot

Write-Host "==== Configure Coverage ====" -ForegroundColor Cyan
cmake --preset coverage

Write-Host "==== Build Coverage ====" -ForegroundColor Cyan
cmake --build --preset coverage

Write-Host "==== Run Tests ====" -ForegroundColor Cyan
ctest --test-dir build/coverage --output-on-failure

Write-Host "==== Generate Coverage Report ====" -ForegroundColor Cyan
python -m gcovr `
    -r . `
    --object-directory build/coverage `
    --html-details `
    -o build/coverage/coverage.html `
    --exclude "third_party/.*" `
    --exclude "test/.*"

Write-Host "==== Open Coverage Report ====" -ForegroundColor Cyan
Start-Process (Resolve-Path "build/coverage/coverage.html")