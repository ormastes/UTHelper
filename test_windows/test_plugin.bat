@echo off
echo ================================
echo UTHelper Plugin Test (Batch)
echo ================================
echo.

echo [Test 1] Loading plugin...
clang++ -Xclang -load -Xclang UTHelperPlugin.dll -fsyntax-only test_simple.cpp
if %ERRORLEVEL% EQU 0 (
    echo OK Plugin loaded successfully
) else (
    echo FAIL Plugin failed to load
    exit /b 1
)

echo.
echo [Test 2] Transforming code...
clang++ -Xclang -load -Xclang UTHelperPlugin.dll -Xclang -plugin -Xclang uthelper -Xclang -plugin-arg-uthelper -Xclang "base-folder=%CD%" -fsyntax-only test_simple.cpp > test_transformed.cpp
if %ERRORLEVEL% EQU 0 (
    echo OK Code transformed successfully
) else (
    echo FAIL Transformation failed
    exit /b 1
)

echo.
echo [Test 3] Compiling transformed code...
clang++ -o test_transformed.exe test_transformed.cpp
if %ERRORLEVEL% EQU 0 (
    echo OK Compilation successful
) else (
    echo FAIL Compilation failed
    exit /b 1
)

echo.
echo [Test 4] Running transformed code...
test_transformed.exe
if %ERRORLEVEL% EQU 0 (
    echo OK Execution successful
) else (
    echo FAIL Execution failed
    exit /b 1
)

echo.
echo ================================
echo All tests passed!
echo ================================
