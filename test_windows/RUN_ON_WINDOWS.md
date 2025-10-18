# Testing UTHelper Plugin on Windows

## Prerequisites
1. Install LLVM/Clang for Windows (version 18 or later)
   - Download from: https://github.com/llvm/llvm-project/releases
   - Or use Clang from llvm-mingw

## Files Needed
1. `UTHelperPlugin.dll` - The plugin (from build-windows/plugin/)
2. `test_simple.cpp` - The test source file

## How to Test

### Test 1: Plugin Loading
```cmd
clang++ -Xclang -load -Xclang UTHelperPlugin.dll -fsyntax-only test_simple.cpp
```

Expected: Should complete without errors (plugin loads successfully)

### Test 2: Transform Code (All Features)
```cmd
clang++ -Xclang -load -Xclang UTHelperPlugin.dll ^
        -Xclang -plugin -Xclang uthelper ^
        -Xclang -plugin-arg-uthelper -Xclang "base-folder=%CD%" ^
        -fsyntax-only test_simple.cpp > test_transformed.cpp
```

Expected output in `test_transformed.cpp`:
- `final` keyword removed from DatabaseHandler
- `virtual` added to all methods
- Friend declarations added to the class

### Test 3: Compile Transformed Code
```cmd
clang++ -o test_transformed.exe test_transformed.cpp
test_transformed.exe
```

Expected: Program compiles and runs successfully

## Verify Transformation

The transformed code should look like:

```cpp
class DatabaseHandler {
  friend class DatabaseHandlerTest;
  template<typename T> friend class DatabaseHandlerFriend;

public:
    virtual void connect() {
        std::cout << "Connecting to database" << std::endl;
    }

    virtual std::string query(const std::string& sql) {
        return "Result: " + sql;
    }

private:
    virtual void authenticate() {
        std::cout << "Authenticating" << std::endl;
    }
};
```

## Success Criteria
- ✓ Plugin loads without errors
- ✓ Code transformation completes
- ✓ `final` keyword is removed
- ✓ `virtual` is added to methods
- ✓ Friend declarations are added
- ✓ Transformed code compiles
- ✓ Transformed code runs correctly

## Troubleshooting

### "Unable to load plugin"
- Make sure clang.exe and UTHelperPlugin.dll are in the same directory, or
- Use absolute path to UTHelperPlugin.dll

### Missing LLVM libraries
- The plugin depends on LLVM libraries built into the DLL
- The DLL is large (~60MB) because it's statically linked

## Automated Test Script (PowerShell)

```powershell
# test_plugin.ps1
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
```

Save this as `test_plugin.ps1` and run:
```powershell
powershell -ExecutionPolicy Bypass .\test_plugin.ps1
```
