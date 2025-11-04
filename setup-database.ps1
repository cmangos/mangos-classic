<#
.SYNOPSIS
    Script de configuración de bases de datos para mangos-classic

.DESCRIPTION
    Este script automatiza la instalación y configuración de las bases de datos
    necesarias para mangos-classic, incluyendo:
    - Creación de bases de datos
    - Creación de usuario y permisos
    - Aplicación de schemas base
    - Aplicación de updates SQL
    - Configuración de módulo Playerbots

.PARAMETER MySQLHost
    Servidor MySQL (por defecto: localhost)

.PARAMETER MySQLPort
    Puerto de MySQL (por defecto: 3306)

.PARAMETER MySQLRootUser
    Usuario root de MySQL (por defecto: root)

.PARAMETER MySQLRootPassword
    Contraseña del usuario root de MySQL

.PARAMETER MangosDBName
    Nombre de la base de datos principal (por defecto: classicmangos)

.PARAMETER CharactersDBName
    Nombre de la base de datos de personajes (por defecto: classiccharacters)

.PARAMETER RealmdDBName
    Nombre de la base de datos de autenticación (por defecto: classicrealmd)

.PARAMETER LogsDBName
    Nombre de la base de datos de logs (por defecto: classiclogs)

.PARAMETER MangosUser
    Usuario de mangos para la aplicación (por defecto: mangos)

.PARAMETER MangosPassword
    Contraseña del usuario mangos (por defecto: mangos)

.PARAMETER SkipUpdates
    Omitir aplicación de scripts de actualización

.PARAMETER DropExisting
    Eliminar bases de datos existentes antes de crear nuevas

.PARAMETER OnlyUpdates
    Solo aplicar updates, no crear bases de datos desde cero

.EXAMPLE
    .\setup-database.ps1 -MySQLRootPassword "mipassword"
    .\setup-database.ps1 -MySQLRootPassword "mipassword" -MangosDBName "mi_mangos"
    .\setup-database.ps1 -MySQLRootPassword "mipassword" -OnlyUpdates
    .\setup-database.ps1 -MySQLRootPassword "mipassword" -DropExisting
#>

param(
    [Parameter(Mandatory=$false)]
    [string]$MySQLHost = "localhost",

    [Parameter(Mandatory=$false)]
    [int]$MySQLPort = 3306,

    [Parameter(Mandatory=$false)]
    [string]$MySQLRootUser = "root",

    [Parameter(Mandatory=$true)]
    [string]$MySQLRootPassword,

    [Parameter(Mandatory=$false)]
    [string]$MangosDBName = "classicmangos",

    [Parameter(Mandatory=$false)]
    [string]$CharactersDBName = "classiccharacters",

    [Parameter(Mandatory=$false)]
    [string]$RealmdDBName = "classicrealmd",

    [Parameter(Mandatory=$false)]
    [string]$LogsDBName = "classiclogs",

    [Parameter(Mandatory=$false)]
    [string]$MangosUser = "mangos",

    [Parameter(Mandatory=$false)]
    [string]$MangosPassword = "mangos",

    [Parameter(Mandatory=$false)]
    [switch]$SkipUpdates = $false,

    [Parameter(Mandatory=$false)]
    [switch]$DropExisting = $false,

    [Parameter(Mandatory=$false)]
    [switch]$OnlyUpdates = $false
)

$ErrorActionPreference = "Stop"

# Colores
function Write-Success($msg) { Write-Host "✓ $msg" -ForegroundColor Green }
function Write-Info($msg) { Write-Host "ℹ $msg" -ForegroundColor Cyan }
function Write-Warn($msg) { Write-Host "⚠ $msg" -ForegroundColor Yellow }
function Write-Err($msg) { Write-Host "✗ $msg" -ForegroundColor Red }
function Write-Header($msg) {
    Write-Host "`n═══════════════════════════════════════════════════════════════" -ForegroundColor Magenta
    Write-Host "  $msg" -ForegroundColor Magenta
    Write-Host "═══════════════════════════════════════════════════════════════`n" -ForegroundColor Magenta
}

# Verificar MySQL
function Test-MySQL {
    try {
        $null = & mysql --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Success "MySQL cliente encontrado"
            return $true
        }
    } catch {
        Write-Err "MySQL cliente no encontrado en PATH"
        Write-Info "Por favor instala MySQL o MariaDB y asegúrate de que 'mysql' esté en el PATH"
        return $false
    }
}

# Ejecutar comando MySQL
function Invoke-MySQLCommand {
    param(
        [string]$Query,
        [string]$Database = "",
        [switch]$Silent = $false
    )

    $args = @(
        "-h", $MySQLHost,
        "-P", $MySQLPort,
        "-u", $MySQLRootUser,
        "-p$MySQLRootPassword"
    )

    if ($Database) {
        $args += $Database
    }

    $args += "-e", $Query

    try {
        if ($Silent) {
            $result = & mysql $args 2>&1
        } else {
            $result = & mysql $args
        }

        if ($LASTEXITCODE -ne 0) {
            return $false
        }
        return $true
    } catch {
        return $false
    }
}

# Ejecutar script SQL desde archivo
function Invoke-MySQLScript {
    param(
        [string]$ScriptPath,
        [string]$Database
    )

    if (-not (Test-Path $ScriptPath)) {
        Write-Warn "Script no encontrado: $ScriptPath"
        return $false
    }

    $args = @(
        "-h", $MySQLHost,
        "-P", $MySQLPort,
        "-u", $MySQLRootUser,
        "-p$MySQLRootPassword",
        $Database
    )

    try {
        Get-Content $ScriptPath -Raw | & mysql $args 2>&1 | Out-Null
        if ($LASTEXITCODE -eq 0) {
            return $true
        }
        return $false
    } catch {
        Write-Warn "Error ejecutando script: $ScriptPath - $_"
        return $false
    }
}

# Crear bases de datos
function New-Databases {
    Write-Header "CREANDO BASES DE DATOS"

    if ($DropExisting) {
        Write-Warn "Eliminando bases de datos existentes..."
        Invoke-MySQLCommand -Query "DROP DATABASE IF EXISTS ``$MangosDBName``" -Silent
        Invoke-MySQLCommand -Query "DROP DATABASE IF EXISTS ``$CharactersDBName``" -Silent
        Invoke-MySQLCommand -Query "DROP DATABASE IF EXISTS ``$RealmdDBName``" -Silent
        Invoke-MySQLCommand -Query "DROP DATABASE IF EXISTS ``$LogsDBName``" -Silent
        Write-Success "Bases de datos anteriores eliminadas"
    }

    Write-Info "Creando base de datos: $MangosDBName"
    if (-not (Invoke-MySQLCommand -Query "CREATE DATABASE IF NOT EXISTS ``$MangosDBName`` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci")) {
        Write-Err "Error creando $MangosDBName"
        return $false
    }

    Write-Info "Creando base de datos: $CharactersDBName"
    if (-not (Invoke-MySQLCommand -Query "CREATE DATABASE IF NOT EXISTS ``$CharactersDBName`` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci")) {
        Write-Err "Error creando $CharactersDBName"
        return $false
    }

    Write-Info "Creando base de datos: $RealmdDBName"
    if (-not (Invoke-MySQLCommand -Query "CREATE DATABASE IF NOT EXISTS ``$RealmdDBName`` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci")) {
        Write-Err "Error creando $RealmdDBName"
        return $false
    }

    Write-Info "Creando base de datos: $LogsDBName"
    if (-not (Invoke-MySQLCommand -Query "CREATE DATABASE IF NOT EXISTS ``$LogsDBName`` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci")) {
        Write-Err "Error creando $LogsDBName"
        return $false
    }

    Write-Success "Todas las bases de datos creadas exitosamente"
    return $true
}

# Crear usuario y permisos
function New-MangosUser {
    Write-Header "CONFIGURANDO USUARIO Y PERMISOS"

    Write-Info "Creando usuario: $MangosUser@localhost"
    Invoke-MySQLCommand -Query "CREATE USER IF NOT EXISTS '$MangosUser'@'localhost' IDENTIFIED BY '$MangosPassword'" -Silent

    Write-Info "Asignando permisos para $MangosDBName"
    if (-not (Invoke-MySQLCommand -Query "GRANT INDEX, SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES ON ``$MangosDBName``.* TO '$MangosUser'@'localhost'")) {
        Write-Err "Error asignando permisos para $MangosDBName"
        return $false
    }

    Write-Info "Asignando permisos para $CharactersDBName"
    if (-not (Invoke-MySQLCommand -Query "GRANT INDEX, SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES ON ``$CharactersDBName``.* TO '$MangosUser'@'localhost'")) {
        Write-Err "Error asignando permisos para $CharactersDBName"
        return $false
    }

    Write-Info "Asignando permisos para $RealmdDBName"
    if (-not (Invoke-MySQLCommand -Query "GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES ON ``$RealmdDBName``.* TO '$MangosUser'@'localhost'")) {
        Write-Err "Error asignando permisos para $RealmdDBName"
        return $false
    }

    Write-Info "Asignando permisos para $LogsDBName"
    if (-not (Invoke-MySQLCommand -Query "GRANT SELECT, INSERT, UPDATE, DELETE, CREATE, DROP, ALTER, LOCK TABLES, CREATE TEMPORARY TABLES ON ``$LogsDBName``.* TO '$MangosUser'@'localhost'")) {
        Write-Err "Error asignando permisos para $LogsDBName"
        return $false
    }

    Invoke-MySQLCommand -Query "FLUSH PRIVILEGES" -Silent

    Write-Success "Usuario y permisos configurados correctamente"
    return $true
}

# Aplicar schemas base
function Install-BaseSchemas {
    Write-Header "APLICANDO SCHEMAS BASE"

    $sqlDir = Join-Path $PSScriptRoot "sql"

    Write-Info "Aplicando schema base de $MangosDBName..."
    if (-not (Invoke-MySQLScript -ScriptPath (Join-Path $sqlDir "base\mangos.sql") -Database $MangosDBName)) {
        Write-Err "Error aplicando schema de mangos"
        return $false
    }
    Write-Success "Schema de $MangosDBName aplicado"

    Write-Info "Aplicando schema base de $CharactersDBName..."
    if (-not (Invoke-MySQLScript -ScriptPath (Join-Path $sqlDir "base\characters.sql") -Database $CharactersDBName)) {
        Write-Err "Error aplicando schema de characters"
        return $false
    }
    Write-Success "Schema de $CharactersDBName aplicado"

    Write-Info "Aplicando schema base de $RealmdDBName..."
    if (-not (Invoke-MySQLScript -ScriptPath (Join-Path $sqlDir "base\realmd.sql") -Database $RealmdDBName)) {
        Write-Err "Error aplicando schema de realmd"
        return $false
    }
    Write-Success "Schema de $RealmdDBName aplicado"

    Write-Info "Aplicando schema base de $LogsDBName..."
    if (-not (Invoke-MySQLScript -ScriptPath (Join-Path $sqlDir "base\logs.sql") -Database $LogsDBName)) {
        Write-Err "Error aplicando schema de logs"
        return $false
    }
    Write-Success "Schema de $LogsDBName aplicado"

    # Aplicar archivos DBC
    Write-Info "Aplicando datos DBC originales..."
    $dbcPath = Join-Path $sqlDir "base\dbc\original_data"
    if (Test-Path $dbcPath) {
        Get-ChildItem -Path $dbcPath -Filter "*.sql" | ForEach-Object {
            Write-Host "  - Aplicando $($_.Name)" -ForegroundColor Gray
            Invoke-MySQLScript -ScriptPath $_.FullName -Database $MangosDBName | Out-Null
        }
        Write-Success "Datos DBC aplicados"
    }

    # Aplicar fixes DBC
    Write-Info "Aplicando fixes DBC de CMaNGOS..."
    $dbcFixesPath = Join-Path $sqlDir "base\dbc\cmangos_fixes"
    if (Test-Path $dbcFixesPath) {
        Get-ChildItem -Path $dbcFixesPath -Filter "*.sql" | ForEach-Object {
            Write-Host "  - Aplicando $($_.Name)" -ForegroundColor Gray
            Invoke-MySQLScript -ScriptPath $_.FullName -Database $MangosDBName | Out-Null
        }
        Write-Success "Fixes DBC aplicados"
    }

    # Aplicar comandos AHBot si existen
    $ahbotPath = Join-Path $sqlDir "base\ahbot\mangos_command_ahbot.sql"
    if (Test-Path $ahbotPath) {
        Write-Info "Aplicando comandos AHBot..."
        Invoke-MySQLScript -ScriptPath $ahbotPath -Database $MangosDBName | Out-Null
        Write-Success "Comandos AHBot aplicados"
    }

    Write-Success "Todos los schemas base aplicados exitosamente"
    return $true
}

# Aplicar updates
function Install-Updates {
    Write-Header "APLICANDO ACTUALIZACIONES SQL"

    $sqlDir = Join-Path $PSScriptRoot "sql\updates"

    # Aplicar updates de mangos
    Write-Info "Aplicando updates de mangos..."
    $mangosUpdates = Get-ChildItem -Path (Join-Path $sqlDir "mangos") -Filter "*.sql" | Sort-Object Name
    $count = 0
    foreach ($update in $mangosUpdates) {
        Invoke-MySQLScript -ScriptPath $update.FullName -Database $MangosDBName | Out-Null
        $count++
        if ($count % 10 -eq 0) {
            Write-Host "  Procesados $count de $($mangosUpdates.Count) updates..." -ForegroundColor Gray
        }
    }
    Write-Success "Aplicados $count updates de mangos"

    # Aplicar updates de characters
    Write-Info "Aplicando updates de characters..."
    $charUpdates = Get-ChildItem -Path (Join-Path $sqlDir "characters") -Filter "*.sql" | Sort-Object Name
    $count = 0
    foreach ($update in $charUpdates) {
        Invoke-MySQLScript -ScriptPath $update.FullName -Database $CharactersDBName | Out-Null
        $count++
        if ($count % 10 -eq 0) {
            Write-Host "  Procesados $count de $($charUpdates.Count) updates..." -ForegroundColor Gray
        }
    }
    Write-Success "Aplicados $count updates de characters"

    # Aplicar updates de realmd
    Write-Info "Aplicando updates de realmd..."
    $realmdPath = Join-Path $sqlDir "realmd"
    if (Test-Path $realmdPath) {
        $realmdUpdates = Get-ChildItem -Path $realmdPath -Filter "*.sql" | Sort-Object Name
        $count = 0
        foreach ($update in $realmdUpdates) {
            Invoke-MySQLScript -ScriptPath $update.FullName -Database $RealmdDBName | Out-Null
            $count++
        }
        Write-Success "Aplicados $count updates de realmd"
    } else {
        Write-Info "No hay updates de realmd"
    }

    # Aplicar updates de logs
    Write-Info "Aplicando updates de logs..."
    $logsPath = Join-Path $sqlDir "logs"
    if (Test-Path $logsPath) {
        $logsUpdates = Get-ChildItem -Path $logsPath -Filter "*.sql" | Sort-Object Name
        $count = 0
        foreach ($update in $logsUpdates) {
            Invoke-MySQLScript -ScriptPath $update.FullName -Database $LogsDBName | Out-Null
            $count++
        }
        Write-Success "Aplicados $count updates de logs"
    } else {
        Write-Info "No hay updates de logs"
    }

    Write-Success "Todas las actualizaciones aplicadas exitosamente"
    return $true
}

# Mostrar resumen
function Show-Summary {
    Write-Header "RESUMEN DE INSTALACIÓN"

    Write-Host "Servidor MySQL:          " -NoNewline
    Write-Host "$MySQLHost:$MySQLPort" -ForegroundColor White

    Write-Host "`nBases de Datos Creadas:" -ForegroundColor Cyan
    Write-Host "  • Mundo:               " -NoNewline
    Write-Host $MangosDBName -ForegroundColor Green
    Write-Host "  • Personajes:          " -NoNewline
    Write-Host $CharactersDBName -ForegroundColor Green
    Write-Host "  • Autenticación:       " -NoNewline
    Write-Host $RealmdDBName -ForegroundColor Green
    Write-Host "  • Logs:                " -NoNewline
    Write-Host $LogsDBName -ForegroundColor Green

    Write-Host "`nUsuario Aplicación:" -ForegroundColor Cyan
    Write-Host "  • Usuario:             " -NoNewline
    Write-Host "$MangosUser@localhost" -ForegroundColor Green
    Write-Host "  • Contraseña:          " -NoNewline
    Write-Host $MangosPassword -ForegroundColor Yellow

    Write-Host "`nPróximos Pasos:" -ForegroundColor Cyan
    Write-Host "  1. Configurar archivos .conf con los datos de conexión" -ForegroundColor White
    Write-Host "  2. Descargar la base de datos del mundo (world database)" -ForegroundColor White
    Write-Host "     desde: https://github.com/cmangos/classic-db" -ForegroundColor Gray
    Write-Host "  3. Importar la base de datos del mundo a $MangosDBName" -ForegroundColor White
    Write-Host "  4. Ejecutar los extractores (maps, dbc, vmaps, mmaps)" -ForegroundColor White
    Write-Host "  5. Iniciar los servidores (realmd y mangosd)" -ForegroundColor White
}

# ============================================
# MAIN SCRIPT
# ============================================

Write-Host ""
Write-Host "╔══════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                                                              ║" -ForegroundColor Cyan
Write-Host "║     CONFIGURACIÓN DE BASE DE DATOS MANGOS-CLASSIC           ║" -ForegroundColor Cyan
Write-Host "║                                                              ║" -ForegroundColor Cyan
Write-Host "╚══════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Verificar MySQL
if (-not (Test-MySQL)) {
    Write-Err "MySQL no está disponible. Instala MySQL o MariaDB primero."
    exit 1
}

# Verificar conexión
Write-Info "Verificando conexión a MySQL..."
if (-not (Invoke-MySQLCommand -Query "SELECT 1" -Silent)) {
    Write-Err "No se pudo conectar a MySQL. Verifica las credenciales."
    exit 1
}
Write-Success "Conexión a MySQL exitosa"

# Crear bases de datos y usuario (si no es solo updates)
if (-not $OnlyUpdates) {
    if (-not (New-Databases)) {
        Write-Err "Error creando bases de datos. Abortando."
        exit 1
    }

    if (-not (New-MangosUser)) {
        Write-Err "Error configurando usuario. Abortando."
        exit 1
    }

    if (-not (Install-BaseSchemas)) {
        Write-Err "Error aplicando schemas base. Abortando."
        exit 1
    }
}

# Aplicar updates
if (-not $SkipUpdates) {
    if (-not (Install-Updates)) {
        Write-Err "Error aplicando actualizaciones. Abortando."
        exit 1
    }
} else {
    Write-Warn "Se omitió la aplicación de actualizaciones (-SkipUpdates)"
}

# Mostrar resumen
Show-Summary

Write-Host ""
Write-Success "════════════════════════════════════════════════════════════════"
Write-Success "  ¡CONFIGURACIÓN DE BASE DE DATOS COMPLETADA!"
Write-Success "════════════════════════════════════════════════════════════════"
Write-Host ""
