<#
.SYNOPSIS
  Build the OpenRide Unreal module on Windows, tee the full output to build.log,
  and print only the error lines.

.EXAMPLE
  .\scripts\build-win.ps1 -EngineDir "C:\Program Files\Epic Games\UE_5.8"
  .\scripts\build-win.ps1 -EngineDir D:\UE_5.8 -Target OpenRide   # Game target
#>
param(
    # Engine root; falls back to $env:UE_ROOT so it can be set once per machine.
    [string]$EngineDir = $env:UE_ROOT,
    [ValidateSet("OpenRideEditor", "OpenRide")]
    [string]$Target = "OpenRideEditor",
    [string]$Configuration = "Development"
)

$ErrorActionPreference = "Stop"
if (-not $EngineDir) { throw "Pass -EngineDir or set UE_ROOT to the Unreal Engine root folder." }

$repo    = Split-Path -Parent $PSScriptRoot
$project = Join-Path $repo "OpenRide.uproject"
$build   = Join-Path $EngineDir "Engine\Build\BatchFiles\Build.bat"
$log     = Join-Path $repo "build.log"
if (-not (Test-Path $build)) { throw "Build.bat not found: $build" }

# Full output goes to build.log; the first error is the one that matters (later ones may cascade).
& $build $Target Win64 $Configuration "-Project=$project" -WaitMutex 2>&1 | Tee-Object -FilePath $log | Out-Null
$exit = $LASTEXITCODE

$errors = Select-String -Path $log -Pattern "error C\d+|fatal error|error LNK|: error " 
if ($errors) { $errors | ForEach-Object { $_.Line } } else { Write-Host "No error lines found." }
Write-Host "Full log: $log (exit code $exit)"
exit $exit
