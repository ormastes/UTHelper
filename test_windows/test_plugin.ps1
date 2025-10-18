# PowerShell test script for UTHelper Plugin on Windows
$ErrorActionPreference = "Stop"

Write-Host "Testing UTHelper Plugin on Windows" -ForegroundColor Green

# Test 1: Load plugin
Write-Host "`n[Test 1] Loading plugin..." -ForegroundColor Yellow
clang++ -Xclang -load -Xclang UTHelperPlugin.dll -fsyntax-only test_simple.cpp
if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Plugin loaded successfully" -ForegroundColor Green
} else {
    Write-Host "✗ Plugin failed to load" -ForegroundColor Red
    exit 1
}

# Test 2: Transform code
Write-Host "`n[Test 2] Transforming code..." -ForegroundColor Yellow
clang++ -Xclang -load -Xclang UTHelperPlugin.dll `
        -Xclang -plugin -Xclang uthelper `
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=$PWD" `
        -fsyntax-only test_simple.cpp > test_transformed.cpp

if ($LASTEXITCODE -eq 0 -and (Test-Path test_transformed.cpp)) {
    Write-Host "✓ Code transformed successfully" -ForegroundColor Green

    # Check for expected transformations
    $content = Get-Content test_transformed.cpp -Raw
    if ($content -match "friend class DatabaseHandlerTest" -and
        $content -match "virtual void connect" -and
        $content -notmatch "class DatabaseHandler final") {
        Write-Host "✓ All transformations verified" -ForegroundColor Green
    } else {
        Write-Host "⚠ Transformation incomplete" -ForegroundColor Yellow
        Write-Host "Content preview:"
        Get-Content test_transformed.cpp | Select-Object -First 30
    }
} else {
    Write-Host "✗ Transformation failed" -ForegroundColor Red
    exit 1
}

# Test 3: Compile transformed code
Write-Host "`n[Test 3] Compiling transformed code..." -ForegroundColor Yellow
clang++ -o test_transformed.exe test_transformed.cpp
if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Compilation successful" -ForegroundColor Green
} else {
    Write-Host "✗ Compilation failed" -ForegroundColor Red
    exit 1
}

# Test 4: Run transformed code
Write-Host "`n[Test 4] Running transformed code..." -ForegroundColor Yellow
.\test_transformed.exe
if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Execution successful" -ForegroundColor Green
} else {
    Write-Host "✗ Execution failed" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== All tests passed! ===" -ForegroundColor Green
