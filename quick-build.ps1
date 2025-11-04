<#
.SYNOPSIS
    Script rápido de compilación (asume que todas las dependencias están instaladas)

.DESCRIPTION
    Este script es una versión simplificada para usuarios que ya tienen todas
    las dependencias instaladas y solo quieren compilar rápidamente.

.PARAMETER BuildType
    Tipo de compilación: Release, Debug, RelWithDebInfo, MinSizeRel (por defecto: Release)

.PARAMETER Clean
    Limpia el directorio de compilación antes de compilar

.EXAMPLE
    .\quick-build.ps1
    .\quick-build.ps1 -BuildType Debug
    .\quick-build.ps1 -Clean
#>

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet('Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$BuildType = 'Release',

    [Parameter(Mandatory=$false)]
    [switch]$Clean = $false,

    [Parameter(Mandatory=$false)]
    [switch]$EnablePlayerbots = $true,

    [Parameter(Mandatory=$false)]
    [switch]$EnableAHBot = $false,

    [Parameter(Mandatory=$false)]
    [switch]$EnableExtractors = $false
)

$ErrorActionPreference = "Stop"

# Colores
function Write-Success($msg) { Write-Host "✓ $msg" -ForegroundColor Green }
function Write-Info($msg) { Write-Host "ℹ $msg" -ForegroundColor Cyan }
function Write-Err($msg) { Write-Host "✗ $msg" -ForegroundColor Red }

Write-Host "`n╔═══════════════════════════════════════╗" -ForegroundColor Magenta
Write-Host "║  COMPILACIÓN RÁPIDA MANGOS-CLASSIC   ║" -ForegroundColor Magenta
Write-Host "╚═══════════════════════════════════════╝`n" -ForegroundColor Magenta

$buildDir = Join-Path $PSScriptRoot "build"

# Limpiar si se solicita
if ($Clean -and (Test-Path $buildDir)) {
    Write-Info "Limpiando directorio de compilación..."
    Remove-Item -Recurse -Force $buildDir
    Write-Success "Directorio limpiado"
}

# Crear directorio de compilación
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
    Write-Info "Directorio de compilación creado"
}

# Configurar
Write-Info "Configurando proyecto ($BuildType)..."
Write-Host "Playerbots: $(if ($EnablePlayerbots) { 'ON' } else { 'OFF' }) | AHBot: $(if ($EnableAHBot) { 'ON' } else { 'OFF' }) | Extractors: $(if ($EnableExtractors) { 'ON' } else { 'OFF' })" -ForegroundColor Cyan

Push-Location $buildDir
try {
    $cmakeArgs = @(
        "..",
        "-DCMAKE_BUILD_TYPE=$BuildType"
    )

    if ($EnablePlayerbots) { $cmakeArgs += "-DBUILD_PLAYERBOTS=ON" }
    if ($EnableAHBot) { $cmakeArgs += "-DBUILD_AHBOT=ON" }
    if ($EnableExtractors) { $cmakeArgs += "-DBUILD_EXTRACTORS=ON" }

    & cmake $cmakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Err "Error en configuración CMake"
        exit 1
    }
    Write-Success "Configuración completada"

    # Compilar
    Write-Info "Compilando..."
    $cores = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
    Write-Info "Usando $cores núcleos"

    cmake --build . --config $BuildType --parallel $cores
    if ($LASTEXITCODE -ne 0) {
        Write-Err "Error en compilación"
        exit 1
    }

    Write-Success "¡Compilación exitosa!`n"

    # Mostrar resultados
    $binPath = "bin\x64_$BuildType"
    if (Test-Path $binPath) {
        Write-Info "Binarios en: $buildDir\$binPath"
    }

} finally {
    Pop-Location
}

Write-Host "`n" -NoNewline
Write-Success "═══════════════════════════════════════"
Write-Success "  ¡LISTO!"
Write-Success "═══════════════════════════════════════"
Write-Host ""
