param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug',
    [switch]$Smoke
)
$ErrorActionPreference = 'Stop'
$solution = Join-Path $PSScriptRoot 'GraphicsSandbox.sln'
if (-not (Test-Path -LiteralPath $solution)) {
    & (Join-Path $PSScriptRoot 'Premake/premake5.exe') "--file=$PSScriptRoot/premake5.lua" vs2022
    if ($LASTEXITCODE -ne 0) { throw 'Project generation failed.' }
}
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Install Visual Studio 2026 with Desktop development with C++ and the v145 toolset.' }
$installation = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -property installationPath
if (-not $installation) { throw 'Cannot locate Visual Studio MSBuild.' }
$msbuild = Join-Path $installation 'MSBuild/Current/Bin/MSBuild.exe'
& $msbuild $solution /m /nologo "/p:Configuration=$Configuration" /p:Platform=x64 /v:minimal /fl "/flp:logfile=$PSScriptRoot/build-$Configuration.log;verbosity=normal"
if ($LASTEXITCODE -ne 0) { throw 'Sandbox build failed.' }
if ($Smoke) {
    $runtime = Join-Path $PSScriptRoot "Bin/$Configuration"
    $process = Start-Process -FilePath (Join-Path $runtime 'Sandbox.exe') -ArgumentList '--smoke' -WorkingDirectory $runtime -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit(60000)) { throw "Smoke check timed out. Inspect Sandbox process $($process.Id)." }
    if ($process.ExitCode -ne 0) { throw "Smoke check failed with exit code $($process.ExitCode). Inspect $runtime/sandbox-error.log and Captures/smoke-report.txt." }
    Get-Content -LiteralPath (Join-Path $runtime 'Captures/smoke-report.txt')
}
