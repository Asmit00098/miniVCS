# SecureVCS Test Runner (PowerShell)
# Usage:
#   .\run_tests.ps1          # Build and run all tests
#   .\run_tests.ps1 -Clean   # Rebuild test runner from scratch

param(
    [switch]$Clean,
    [switch]$Help
)

if ($Help) {
    Write-Host "SecureVCS Test Runner" -ForegroundColor Cyan
    Write-Host "Usage: .\run_tests.ps1 [-Clean] [-Help]"
    exit 0
}

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

if (!(Test-Path "bin")) {
    New-Item -ItemType Directory -Path "bin" | Out-Null
}

$TestRunner = "bin\test_runner.exe"

$NeedBuild = $Clean -or (!(Test-Path $TestRunner))

if (-not $NeedBuild) {
    # Check if any source or test file is newer than test_runner.exe
    $RunnerTime = (Get-Item $TestRunner).LastWriteTime
    $SourceFiles = Get-ChildItem -Path "main.cpp", "minivcs.hpp", "tests" -Recurse -Include *.cpp, *.hpp
    foreach ($f in $SourceFiles) {
        if ($f.LastWriteTime -gt $RunnerTime) {
            $NeedBuild = $true
            break
        }
    }
}

if ($NeedBuild) {
    Write-Host "Compiling test suite (C++17)..." -ForegroundColor Yellow
    $CompileArgs = @(
        "-std=c++17",
        "-O2",
        "-I.",
        "-DMINIVCS_TEST_BUILD",
        "-DMINIVCS_NO_OPENSSL",
        "main.cpp",
        "tests/test_main.cpp",
        "tests/unit/test_sha256.cpp",
        "tests/unit/test_security.cpp",
        "tests/unit/test_diff.cpp",
        "tests/unit/test_objects.cpp",
        "tests/unit/test_serialization.cpp",
        "tests/unit/test_config.cpp",
        "tests/unit/test_ai.cpp",
        "tests/integration/test_cli.cpp",
        "tests/integration/test_sync.cpp",
        "tests/integration/test_ai_live.cpp",
        "-lws2_32",
        "-o", $TestRunner
    )
    & g++ $CompileArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Write-Host "Build successful.`n" -ForegroundColor Green
}

Write-Host "Running SecureVCS Test Suite..." -ForegroundColor Cyan
& ".\$TestRunner"
exit $LASTEXITCODE
