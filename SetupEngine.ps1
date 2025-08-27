<#
SetupEngine.ps1
Builds CSC581_GameEngine-core-graphics on Windows.

What it does:
  1) Ensures Git and CMake are available (installs Git via winget if needed).
  2) Clones SDL into third_party\SDL (or updates it). Use -ForceReclone to wipe & reclone.
  3) Configures CMake into .\build (uses -Generator if provided).
  4) Builds Debug config.
  5) Locates and launches the target exe (default: GameEngine.exe).

Usage (from repo root):
  powershell -ExecutionPolicy Bypass -File .\SetupEngine.ps1
  powershell -ExecutionPolicy Bypass -File .\SetupEngine.ps1 -ForceReclone
  powershell -ExecutionPolicy Bypass -File .\SetupEngine.ps1 -Generator "Visual Studio 17 2022"
  powershell -ExecutionPolicy Bypass -File .\SetupEngine.ps1 -Target "MyCustomExe.exe"
#>

[CmdletBinding()]
param(
  [switch]$ForceReclone,
  [string]$Generator,
  [string]$Target = "GameEngine.exe"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Test-Command {
  param([Parameter(Mandatory=$true)][string]$Name)
  $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

function Test-Git {
  if (Test-Command -Name "git") {
    Write-Host "Git found: $((git --version))"
    return
  }
  Write-Warning "Git not found. Attempting install via winget."
  if (-not (Test-Command -Name "winget")) {
    throw "winget is not available. Install Git manually (https://git-scm.com/download/win) and re-run."
  }
  & winget install --id Git.Git -e --source winget --silent | Out-Null
  if (-not (Test-Command -Name "git")) {
    throw "Git still not available on PATH after install. Open a new terminal and try again."
  }
  Write-Host "Git installed."
}

function Test-CMake {
  if (Test-Command -Name "cmake") {
    Write-Host "CMake found: $((cmake --version | Select-Object -First 1))"
    return
  }
  throw "CMake is required. Install from https://cmake.org/download/ or via winget: winget install Kitware.CMake"
}

function New-Dir {
  param([string]$Path)
  if (-not (Test-Path $Path)) { New-Item -ItemType Directory -Path $Path | Out-Null }
}

# --- Script start ---
$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $RepoRoot
Write-Host "Repo root: $RepoRoot"

Test-Git
Test-CMake

$thirdParty = Join-Path $RepoRoot "third_party"
New-Dir $thirdParty

# Clone or update SDL
$SDLDir = Join-Path $thirdParty "SDL"
if (Test-Path $SDLDir) {
  if ($ForceReclone) {
    Write-Host "Removing existing $SDLDir due to -ForceReclone"
    Remove-Item -Recurse -Force $SDLDir
  } else {
    Write-Host "SDL already present. Fetching latest..."
    Push-Location $SDLDir
    git fetch --all --tags
    git pull --ff-only
    Pop-Location
  }
}
if (-not (Test-Path $SDLDir)) {
  Write-Host "Cloning SDL into $SDLDir"
  git clone https://github.com/libsdl-org/SDL.git $SDLDir
}

# Configure CMake
$buildDir = Join-Path $RepoRoot "build"
New-Dir $buildDir

$configureArgs = @("-S", $RepoRoot, "-B", $buildDir, "-DUSE_VENDORED_SDL=ON")
if ($Generator -and $Generator.Trim().Length -gt 0) {
  $configureArgs += @("-G", $Generator)
}

Write-Host "`n=== CMake configure ==="
cmake @configureArgs

# Build (always ensure Debug exists; multi-config generators will honor --config)
Write-Host "`n=== CMake build (Debug) ==="
cmake --build $buildDir --config Debug

# Try to locate the produced executable robustly across generators
$exeCandidates = @(
  (Join-Path $buildDir "bin\Debug\$Target"),     # VS multi-config (with RUNTIME_OUTPUT)
  (Join-Path $buildDir "engine\Debug\$Target"),  # default VS layout
  (Join-Path $buildDir "Debug\$Target"),         # some custom layouts
  (Join-Path $buildDir $Target)                  # single-config (Ninja/MinGW)
)
$exe = $exeCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $exe) {
  # last resort: search
  $found = Get-ChildItem -Recurse -Filter $Target -Path $buildDir -ErrorAction SilentlyContinue | Select-Object -First 1
  if ($found) { $exe = $found.FullName }
}

if ($exe -and (Test-Path $exe)) {
  Write-Host "`n=== Running $exe ==="
  & $exe
} else {
  Write-Warning "Could not find $Target after build. Ensure your CMake target matches or pass -Target to this script."
}
