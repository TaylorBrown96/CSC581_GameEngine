param(
  [int]$Port = 7777
)

# Paths (Debug config)
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$ServerExe = Join-Path $Root "build\server_simple_build\Debug\simple_server.exe"
$ClientExe = Join-Path $Root "build\Debug\NetClientDemo.exe"

if (!(Test-Path $ServerExe)) { Write-Host "Server not found at $ServerExe"; exit 1 }
if (!(Test-Path $ClientExe)) { Write-Host "Client not found at $ClientExe"; exit 1 }

# Start Server (new PowerShell window)
Start-Process powershell -ArgumentList @(
  "-NoExit",
  "-Command",
  "cd `"$($ServerExe | Split-Path)`"; .\simple_server.exe $Port"
)

Start-Sleep -Seconds 1  # small delay so the server starts first

# Start Client 1 (new PowerShell window)
Start-Process powershell -ArgumentList @(
  "-NoExit",
  "-Command",
  "cd `"$($ClientExe | Split-Path)`"; .\NetClientDemo.exe --host 127.0.0.1 --port $Port"
)

# Start Client 2 (new PowerShell window)
Start-Process powershell -ArgumentList @(
  "-NoExit",
  "-Command",
  "cd `"$($ClientExe | Split-Path)`"; .\NetClientDemo.exe --host 127.0.0.1 --port $Port"
)
