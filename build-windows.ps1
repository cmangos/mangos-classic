<#
.SYNOPSIS
    Script automatizado para compilar mangos-classic en Windows 11

.DESCRIPTION
    Este script verifica, instala (si es necesario) y configura todas las dependencias
    necesarias para compilar mangos-classic en Windows 11.

.PARAMETER BuildType
    Tipo de compilación: Release, Debug, RelWithDebInfo, MinSizeRel (por defecto: Release)

.PARAMETER SkipDependencyCheck
    Omite la verificación e instalación de dependencias

.PARAMETER BuildDir
    Directorio donde se compilará el proyecto (por defecto: ./build)

.EXAMPLE
    .\build-windows.ps1
    .\build-windows.ps1 -BuildType Debug
    .\build-windows.ps1 -SkipDependencyCheck
#>

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet('Release', 'Debug', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$BuildType = 'Release',

    [Parameter(Mandatory=$false)]
    [switch]$SkipDependencyCheck = $false,

    [Parameter(Mandatory=$false)]
    [string]$BuildDir = "build"
)

# Colores para la salida
function Write-ColorOutput($ForegroundColor) {
    $fc = $host.UI.RawUI.ForegroundColor
    $host.UI.RawUI.ForegroundColor = $ForegroundColor
    if ($args) {
        Write-Output $args
    }
    $host.UI.RawUI.ForegroundColor = $fc
}

function Write-Success($message) {
    Write-ColorOutput Green "✓ $message"
}

function Write-Info($message) {
    Write-ColorOutput Cyan "ℹ $message"
}

function Write-Warning($message) {
    Write-ColorOutput Yellow "⚠ $message"
}

function Write-Error-Custom($message) {
    Write-ColorOutput Red "✗ $message"
}

function Write-Header($message) {
    Write-Host ""
    Write-ColorOutput Magenta "═══════════════════════════════════════════════════════════════"
    Write-ColorOutput Magenta "  $message"
    Write-ColorOutput Magenta "═══════════════════════════════════════════════════════════════"
    Write-Host ""
}

# Verificar que se ejecuta como administrador
function Test-IsAdmin {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Verificar Git
function Test-Git {
    try {
        $gitVersion = git --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Git encontrado: $gitVersion"
            return $true
        }
    } catch {
        Write-Warning "Git no encontrado"
        return $false
    }
}

# Instalar Git
function Install-Git {
    Write-Info "Instalando Git..."
    choco install git -y
    refreshenv
    if (Test-Git) {
        Write-Success "Git instalado correctamente"
        return $true
    } else {
        Write-Error-Custom "Error al instalar Git"
        return $false
    }
}

# Verificar CMake
function Test-CMake {
    try {
        $cmakeVersion = cmake --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            # Extraer versión
            $versionLine = ($cmakeVersion | Select-Object -First 1) -replace 'cmake version ', ''
            $version = [version]($versionLine.Split(' ')[0])
            $minVersion = [version]"3.12.0"

            if ($version -ge $minVersion) {
                Write-Success "CMake encontrado: $versionLine (mínimo requerido: 3.12)"
                return $true
            } else {
                Write-Warning "CMake versión $versionLine es antigua. Se requiere 3.12 o superior"
                return $false
            }
        }
    } catch {
        Write-Warning "CMake no encontrado"
        return $false
    }
}

# Instalar CMake
function Install-CMake {
    Write-Info "Instalando CMake..."
    choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System' -y
    refreshenv
    if (Test-CMake) {
        Write-Success "CMake instalado correctamente"
        return $true
    } else {
        Write-Error-Custom "Error al instalar CMake"
        return $false
    }
}

# Verificar Visual Studio
function Test-VisualStudio {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    if (Test-Path $vsWhere) {
        $vsInstances = & $vsWhere -version "[17.0,18.0)" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

        if ($vsInstances) {
            Write-Success "Visual Studio 2022 encontrado"

            # Verificar C++ workload
            $vcToolsPath = Join-Path $vsInstances "VC\Tools\MSVC"
            if (Test-Path $vcToolsPath) {
                Write-Success "Herramientas de C++ encontradas"
                return $true
            } else {
                Write-Warning "Visual Studio encontrado pero falta el workload de C++"
                return $false
            }
        }
    }

    Write-Warning "Visual Studio 2022 no encontrado"
    return $false
}

# Verificar Chocolatey
function Test-Chocolatey {
    try {
        $chocoVersion = choco --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Chocolatey encontrado: v$chocoVersion"
            return $true
        }
    } catch {
        Write-Warning "Chocolatey no encontrado"
        return $false
    }
}

# Instalar Chocolatey
function Install-Chocolatey {
    Write-Info "Instalando Chocolatey..."
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

    # Actualizar la sesión de PowerShell
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")

    if (Test-Chocolatey) {
        Write-Success "Chocolatey instalado correctamente"
        return $true
    } else {
        Write-Error-Custom "Error al instalar Chocolatey"
        return $false
    }
}

# Verificar Boost
function Test-Boost {
    if ($env:BOOST_ROOT -and (Test-Path $env:BOOST_ROOT)) {
        Write-Success "Boost encontrado en: $env:BOOST_ROOT"
        return $true
    } else {
        Write-Warning "Boost no encontrado (variable BOOST_ROOT no configurada o directorio no existe)"
        return $false
    }
}

# Instalar Boost
function Install-Boost {
    Write-Info "Instalando Boost (esto puede tardar varios minutos)..."
    choco install boost-msvc-14.3 -y

    # Actualizar variables de entorno
    refreshenv

    # Buscar instalación de Boost
    $boostPath = "C:\local\boost_*"
    $boostDirs = Get-ChildItem -Path "C:\local\" -Filter "boost_*" -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending

    if ($boostDirs) {
        $latestBoost = $boostDirs[0].FullName
        [System.Environment]::SetEnvironmentVariable("BOOST_ROOT", $latestBoost, "Machine")
        $env:BOOST_ROOT = $latestBoost
        Write-Success "Boost instalado en: $latestBoost"
        return $true
    } else {
        Write-Error-Custom "Error: No se pudo encontrar la instalación de Boost"
        return $false
    }
}

# Función para refrescar variables de entorno
function refreshenv {
    $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
    $env:BOOST_ROOT = [System.Environment]::GetEnvironmentVariable("BOOST_ROOT","Machine")
}

# Verificar dependencias
function Test-Dependencies {
    Write-Header "VERIFICANDO DEPENDENCIAS"

    $allDepsOk = $true
    $needsRestart = $false

    # Verificar si se ejecuta como administrador
    if (-not (Test-IsAdmin)) {
        Write-Error-Custom "Este script necesita ejecutarse como Administrador para instalar dependencias"
        Write-Info "Por favor, ejecuta PowerShell como Administrador y vuelve a ejecutar el script"
        return $false
    }

    # Verificar e instalar Chocolatey primero
    if (-not (Test-Chocolatey)) {
        if (-not (Install-Chocolatey)) {
            Write-Error-Custom "No se pudo instalar Chocolatey. Instalación manual requerida."
            return $false
        }
        $needsRestart = $true
    }

    # Verificar Git
    if (-not (Test-Git)) {
        if (-not (Install-Git)) {
            $allDepsOk = $false
        } else {
            $needsRestart = $true
        }
    }

    # Verificar CMake
    if (-not (Test-CMake)) {
        if (-not (Install-CMake)) {
            $allDepsOk = $false
        } else {
            $needsRestart = $true
        }
    }

    # Verificar Visual Studio
    if (-not (Test-VisualStudio)) {
        Write-Warning "Visual Studio 2022 no está instalado o no tiene el workload de C++"
        Write-Info "Necesitas instalar Visual Studio 2022 manualmente con:"
        Write-Info "  - Desktop development with C++"
        Write-Info "  - MSVC v143 - VS 2022 C++ x64/x86 build tools"
        Write-Info "  - Windows 11 SDK"
        Write-Info ""
        Write-Info "Descarga: https://visualstudio.microsoft.com/es/downloads/"
        Write-Info "Puedes usar la Community Edition (gratuita)"
        $allDepsOk = $false
    }

    # Verificar Boost
    if (-not (Test-Boost)) {
        if (-not (Install-Boost)) {
            $allDepsOk = $false
        } else {
            $needsRestart = $true
        }
    }

    if ($needsRestart) {
        Write-Warning "Se han instalado nuevas herramientas. Es recomendable cerrar y reabrir PowerShell."
        Write-Info "Presiona Enter para continuar de todas formas o Ctrl+C para salir..."
        Read-Host
        refreshenv
    }

    return $allDepsOk
}

# Configurar el proyecto con CMake
function Invoke-CMakeConfigure {
    param([string]$buildDir, [string]$buildType)

    Write-Header "CONFIGURANDO PROYECTO CON CMAKE"

    $sourceDir = $PSScriptRoot

    # Crear directorio de compilación si no existe
    if (-not (Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
        Write-Info "Directorio de compilación creado: $buildDir"
    }

    # Ejecutar CMake
    Write-Info "Ejecutando CMake..."
    Write-Info "Directorio fuente: $sourceDir"
    Write-Info "Directorio de compilación: $buildDir"
    Write-Info "Tipo de compilación: $buildType"

    Push-Location $buildDir
    try {
        cmake -S $sourceDir -B . -DCMAKE_BUILD_TYPE=$buildType

        if ($LASTEXITCODE -eq 0) {
            Write-Success "Configuración de CMake completada exitosamente"
            return $true
        } else {
            Write-Error-Custom "Error en la configuración de CMake"
            return $false
        }
    } finally {
        Pop-Location
    }
}

# Compilar el proyecto
function Invoke-Build {
    param([string]$buildDir, [string]$buildType)

    Write-Header "COMPILANDO PROYECTO"

    Push-Location $buildDir
    try {
        Write-Info "Iniciando compilación (esto puede tardar varios minutos)..."

        # Usar todos los núcleos disponibles
        $cores = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
        Write-Info "Usando $cores núcleos para la compilación"

        cmake --build . --config $buildType --parallel $cores

        if ($LASTEXITCODE -eq 0) {
            Write-Success "¡Compilación completada exitosamente!"
            return $true
        } else {
            Write-Error-Custom "Error durante la compilación"
            return $false
        }
    } finally {
        Pop-Location
    }
}

# Mostrar información de los binarios generados
function Show-BuildResults {
    param([string]$buildDir, [string]$buildType)

    Write-Header "RESULTADOS DE LA COMPILACIÓN"

    $binPath = Join-Path $buildDir "bin\x64_$buildType"

    if (Test-Path $binPath) {
        Write-Success "Binarios generados en: $binPath"
        Write-Host ""
        Write-Info "Archivos generados:"
        Get-ChildItem -Path $binPath -File | ForEach-Object {
            Write-Host "  - $($_.Name)" -ForegroundColor White
        }
    } else {
        Write-Warning "No se encontró el directorio de binarios esperado: $binPath"
    }
}

# ============================================
# MAIN SCRIPT
# ============================================

Write-Host ""
Write-ColorOutput Cyan @"
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║          SCRIPT DE COMPILACIÓN MANGOS-CLASSIC               ║
║                   Para Windows 11                           ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝
"@
Write-Host ""

# Verificar dependencias
if (-not $SkipDependencyCheck) {
    if (-not (Test-Dependencies)) {
        Write-Host ""
        Write-Error-Custom "ERROR: No se pudieron verificar/instalar todas las dependencias"
        Write-Info "Revisa los errores arriba e instala manualmente lo que falte"
        Write-Info "Luego ejecuta el script nuevamente con -SkipDependencyCheck"
        exit 1
    }
} else {
    Write-Warning "Se omitió la verificación de dependencias (-SkipDependencyCheck)"
}

# Obtener la ruta completa del directorio de compilación
$buildDirFull = Join-Path $PSScriptRoot $BuildDir

# Configurar con CMake
if (-not (Invoke-CMakeConfigure -buildDir $buildDirFull -buildType $BuildType)) {
    Write-Error-Custom "Error en la configuración. Abortando."
    exit 1
}

# Compilar
if (-not (Invoke-Build -buildDir $buildDirFull -buildType $BuildType)) {
    Write-Error-Custom "Error en la compilación. Abortando."
    exit 1
}

# Mostrar resultados
Show-BuildResults -buildDir $buildDirFull -buildType $BuildType

Write-Host ""
Write-Success "════════════════════════════════════════════════════════════════"
Write-Success "  ¡PROCESO COMPLETADO EXITOSAMENTE!"
Write-Success "════════════════════════════════════════════════════════════════"
Write-Host ""
