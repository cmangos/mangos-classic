# 🎮 Guía Completa de Configuración - mangos-classic (Windows 11)

Esta guía completa te llevará desde cero hasta tener un servidor mangos-classic funcionando en Windows 11 con soporte para **Playerbots** (bots controlados por IA).

## 📚 Índice

1. [Vista General](#vista-general)
2. [Proceso Completo de Instalación](#proceso-completo-de-instalación)
3. [Scripts Disponibles](#scripts-disponibles)
4. [Arquitectura del Sistema](#arquitectura-del-sistema)
5. [Preguntas Frecuentes](#preguntas-frecuentes)

---

## 🎯 Vista General

### ¿Qué es mangos-classic?

**mangos-classic** es un emulador de servidor para World of Warcraft Classic (versión 1.12.1). Te permite ejecutar tu propio servidor privado para experimentar WoW Classic.

### ¿Qué incluye esta configuración?

Esta configuración automática incluye:

✅ **Compilación automatizada** con todas las dependencias
✅ **Soporte para Playerbots** (juega con bots IA)
✅ **Configuración completa de base de datos** (4 bases de datos)
✅ **Scripts SQL actualizados** aplicados automáticamente
✅ **Opción de Auction House Bot** (AHBot)
✅ **Herramientas de extracción** (maps, vmaps, mmaps)
✅ **Documentación completa en español**

### Componentes del Servidor

```
┌─────────────────────────────────────────────────────────┐
│                  SERVIDOR MANGOS-CLASSIC                │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────────┐        ┌────────────────────┐       │
│  │   realmd     │◄──────►│  classicrealmd DB  │       │
│  │ (Auth Server)│        │  (Autenticación)   │       │
│  └──────────────┘        └────────────────────┘       │
│                                                         │
│  ┌──────────────┐        ┌────────────────────┐       │
│  │   mangosd    │◄──────►│  classicmangos DB  │       │
│  │ (Game Server)│        │  (Datos del Mundo) │       │
│  │              │        └────────────────────┘       │
│  │  + Playerbots│                                      │
│  │  + AHBot     │        ┌────────────────────┐       │
│  │              │◄──────►│classiccharacters DB│       │
│  └──────────────┘        │   (Personajes)     │       │
│                          └────────────────────┘       │
│                                                         │
│                          ┌────────────────────┐       │
│                          │   classiclogs DB   │       │
│                          │      (Logs)        │       │
│                          └────────────────────┘       │
└─────────────────────────────────────────────────────────┘
```

---

## 🚀 Proceso Completo de Instalación

### Paso 1: Requisitos Previos (Instalación Manual)

#### 1.1 Instalar Visual Studio 2022

**OBLIGATORIO** - Debe hacerse manualmente:

1. Descargar: [Visual Studio 2022 Community](https://visualstudio.microsoft.com/es/downloads/)
2. Durante la instalación seleccionar:
   - ✅ **Desarrollo para el escritorio con C++**
   - ✅ **MSVC v143 build tools**
   - ✅ **Windows 11 SDK**

#### 1.2 Instalar MySQL o MariaDB

Elige una opción:

**Opción A: MySQL (Recomendado)**
```powershell
choco install mysql -y
```

**Opción B: MariaDB**
```powershell
choco install mariadb -y
```

Anota la contraseña root que configures durante la instalación.

---

### Paso 2: Clonar el Repositorio

```powershell
git clone https://github.com/TU_USUARIO/mangos-classic.git
cd mangos-classic
```

---

### Paso 3: Compilar el Servidor

Abre **PowerShell como Administrador**:

```powershell
# Permitir ejecución de scripts (solo la primera vez)
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

# Compilar con Playerbots (por defecto)
.\build-windows.ps1
```

**Esto tomará 15-30 minutos** dependiendo de tu PC.

#### Opciones de Compilación

```powershell
# Solo servidor básico (sin bots)
.\build-windows.ps1 -EnablePlayerbots:$false

# Con Playerbots + AHBot
.\build-windows.ps1 -EnableAHBot

# Con todo (Playerbots + AHBot + Extractors)
.\build-windows.ps1 -EnablePlayerbots -EnableAHBot -EnableExtractors

# Compilación Debug
.\build-windows.ps1 -BuildType Debug
```

**Resultado:** Binarios en `build\bin\x64_Release\`

---

### Paso 4: Configurar Base de Datos

```powershell
.\setup-database.ps1 -MySQLRootPassword "TU_PASSWORD_ROOT_MYSQL"
```

**Esto creará:**
- ✅ 4 bases de datos (classicmangos, classiccharacters, classicrealmd, classiclogs)
- ✅ Usuario `mangos` con contraseña `mangos`
- ✅ Todos los schemas base
- ✅ Todos los updates SQL aplicados
- ✅ Tabla `playerbot_saved_data` para bots

#### Personalizar Base de Datos

```powershell
# Cambiar nombres de DB
.\setup-database.ps1 -MySQLRootPassword "pass" `
    -MangosDBName "mi_mangos" `
    -CharactersDBName "mi_chars"

# Cambiar usuario/contraseña de mangos
.\setup-database.ps1 -MySQLRootPassword "pass" `
    -MangosUser "servidor" `
    -MangosPassword "password_segura"

# Reinstalar (borrar DBs existentes)
.\setup-database.ps1 -MySQLRootPassword "pass" -DropExisting
```

---

### Paso 5: Descargar Datos del Mundo

El servidor necesita los datos del mundo (NPCs, objetos, quests, etc.).

#### Opción A: Usando Classic-DB (Recomendado)

```powershell
# Clonar classic-db
git clone https://github.com/cmangos/classic-db.git
cd classic-db

# Editar InstallFullDB.config con tus datos:
# MYSQL_HOST="localhost"
# MYSQL_USERNAME="root"
# MYSQL_PASSWORD="tu_password"
# MYSQL_DATABASE="classicmangos"
# CORE_PATH="C:/ruta/a/mangos-classic"

# Instalar (requiere Git Bash en Windows)
bash InstallFullDB.sh
```

#### Opción B: Importación Manual

Si tienes un dump SQL:

```powershell
mysql -h localhost -u root -p classicmangos < world_database.sql
```

---

### Paso 6: Extraer Datos del Cliente

**IMPORTANTE:** Necesitas una instalación válida del cliente WoW 1.12.1

Si compilaste con `-EnableExtractors`:

```powershell
cd build\bin\x64_Release

# Copiar estos archivos a tu directorio del cliente WoW:
# - ExtractResources.sh
# - MoveMapGen.exe
# - vmap_assembler.exe
# - vmap_extractor.exe

# En el directorio del cliente WoW, ejecutar:
bash ExtractResources.sh
```

Esto generará:
- `dbc/` - Datos del cliente
- `maps/` - Datos de mapas
- `vmaps/` - Visual maps
- `mmaps/` - Movement maps (para IA de bots)

**Copiar estas carpetas** a `build\bin\x64_Release\`

---

### Paso 7: Configurar Archivos .conf

#### 7.1 Configurar mangosd.conf

Ubicación: `build\bin\x64_Release\mangosd.conf`

**Configuración de Base de Datos:**
```ini
# Buscar estas líneas y configurar:
WorldDatabaseInfo = "localhost;3306;mangos;mangos;classicmangos"
CharacterDatabaseInfo = "localhost;3306;mangos;mangos;classiccharacters"
LogsDatabaseInfo = "localhost;3306;mangos;mangos;classiclogs"
```

**Configuración de DataDir:**
```ini
DataDir = "."
```

**Configuración de Playerbots (si está habilitado):**
```ini
# La configuración de bots está en playerbot.conf
```

#### 7.2 Configurar realmd.conf

Ubicación: `build\bin\x64_Release\realmd.conf`

```ini
LoginDatabaseInfo = "localhost;3306;mangos;mangos;classicrealmd"
```

#### 7.3 Configurar playerbot.conf (si compilaste con Playerbots)

Ubicación: `build\bin\x64_Release\playerbot.conf`

```ini
# Número máximo de bots por jugador (0-9)
PlayerbotAI.MaxNumBots = 9

# Habilitar mensajes de debug
PlayerbotAI.DebugWhisper = 0

# Distancia de seguimiento
PlayerbotAI.FollowDistanceMin = 0.5
PlayerbotAI.FollowDistanceMax = 1.0
```

---

### Paso 8: Crear Cuenta de Usuario

```powershell
# Conectar a la base de datos
mysql -h localhost -u mangos -pmangos classicrealmd

# Crear cuenta (ajusta username y password)
INSERT INTO account (username, sha_pass_hash, gmlevel, email)
VALUES ('admin', SHA1(CONCAT(UPPER('admin'), ':', UPPER('admin'))), 3, 'admin@localhost');

# Salir
exit
```

**Nota:**
- `gmlevel 3` = Administrador
- `gmlevel 0` = Jugador normal

---

### Paso 9: Iniciar el Servidor

```powershell
cd build\bin\x64_Release

# Terminal 1 - Servidor de Autenticación
.\realmd.exe

# Terminal 2 - Servidor de Juego
.\mangosd.exe
```

**Primera vez:** `mangosd.exe` creará archivos `.conf` por defecto si no existen.

---

### Paso 10: Configurar Cliente WoW

1. **Editar `realmlist.wtf`** en tu carpeta de WoW:
   ```
   set realmlist 127.0.0.1
   ```

2. **Iniciar WoW** y conectar con:
   - Usuario: `admin`
   - Contraseña: `admin`

---

## 📜 Scripts Disponibles

### build-windows.ps1

Script principal de compilación con instalación automática de dependencias.

```powershell
# Uso básico
.\build-windows.ps1

# Parámetros disponibles:
-BuildType [Release|Debug|RelWithDebInfo|MinSizeRel]  # Tipo de compilación
-EnablePlayerbots                                      # Habilitar bots (por defecto: true)
-EnableAHBot                                           # Habilitar Auction House Bot
-EnableExtractors                                      # Compilar extractores
-SkipDependencyCheck                                   # Saltar verificación de dependencias
-BuildDir "ruta"                                       # Directorio de compilación personalizado

# Ejemplos:
.\build-windows.ps1 -BuildType Debug -EnableAHBot
.\build-windows.ps1 -EnablePlayerbots:$false
.\build-windows.ps1 -BuildDir "mi-build"
```

**Instala automáticamente:**
- Chocolatey
- Git
- CMake 3.12+
- Boost (boost-msvc-14.3)

**Requiere instalación manual:**
- Visual Studio 2022 con C++ Desktop Development

---

### quick-build.ps1

Compilación rápida sin verificación de dependencias.

```powershell
# Uso básico
.\quick-build.ps1

# Parámetros:
-BuildType [Release|Debug|RelWithDebInfo|MinSizeRel]
-Clean                                                 # Limpiar antes de compilar
-EnablePlayerbots
-EnableAHBot
-EnableExtractors

# Ejemplos:
.\quick-build.ps1 -Clean
.\quick-build.ps1 -BuildType Debug -EnableAHBot
```

---

### setup-database.ps1

Configuración completa de base de datos.

```powershell
# Uso básico (REQUERIDO)
.\setup-database.ps1 -MySQLRootPassword "tu_password"

# Parámetros disponibles:
-MySQLHost "host"                    # Servidor MySQL (default: localhost)
-MySQLPort 3306                      # Puerto (default: 3306)
-MySQLRootUser "root"                # Usuario root (default: root)
-MySQLRootPassword "pass"            # Contraseña root (REQUERIDO)
-MangosDBName "nombre"               # Nombre DB mundo (default: classicmangos)
-CharactersDBName "nombre"           # Nombre DB chars (default: classiccharacters)
-RealmdDBName "nombre"               # Nombre DB auth (default: classicrealmd)
-LogsDBName "nombre"                 # Nombre DB logs (default: classiclogs)
-MangosUser "usuario"                # Usuario app (default: mangos)
-MangosPassword "pass"               # Password app (default: mangos)
-DropExisting                        # Borrar DBs existentes
-OnlyUpdates                         # Solo aplicar updates
-SkipUpdates                         # No aplicar updates

# Ejemplos:
.\setup-database.ps1 -MySQLRootPassword "pass" -DropExisting
.\setup-database.ps1 -MySQLRootPassword "pass" -MangosUser "servidor" -MangosPassword "secure123"
.\setup-database.ps1 -MySQLRootPassword "pass" -OnlyUpdates
```

**Crea y configura:**
- 4 bases de datos
- Usuario de aplicación con permisos
- Schemas base completos
- Todos los SQL updates
- Soporte para Playerbots
- Datos DBC
- Comandos AHBot (si aplicable)

---

## 🏗️ Arquitectura del Sistema

### Directorios Principales

```
mangos-classic/
├── build/                          # Directorio de compilación
│   └── bin/
│       └── x64_Release/           # Binarios compilados
│           ├── mangosd.exe        # Servidor de juego
│           ├── realmd.exe         # Servidor de auth
│           ├── mangosd.conf       # Config servidor juego
│           ├── realmd.conf        # Config servidor auth
│           ├── playerbot.conf     # Config bots (si habilitado)
│           ├── dbc/               # Datos del cliente (extraídos)
│           ├── maps/              # Mapas (extraídos)
│           ├── vmaps/             # Visual maps (extraídos)
│           └── mmaps/             # Movement maps (extraídos)
│
├── sql/                           # Scripts SQL
│   ├── base/                      # Schemas base
│   │   ├── mangos.sql
│   │   ├── characters.sql
│   │   ├── realmd.sql
│   │   ├── logs.sql
│   │   └── dbc/                   # Datos DBC
│   ├── updates/                   # Updates SQL
│   │   ├── mangos/
│   │   ├── characters/
│   │   ├── realmd/
│   │   └── logs/
│   └── create/                    # Scripts de creación
│
├── src/                           # Código fuente
│   ├── game/                      # Lógica del juego
│   │   └── PlayerBot/            # Módulo Playerbots legacy
│   ├── modules/                   # Módulos externos
│   │   └── PlayerBots/           # Módulo Playerbots moderno (descargado)
│   └── shared/                    # Código compartido
│
├── doc/                           # Documentación
│   └── PlayerBot/                # Docs de Playerbots
│
├── build-windows.ps1              # Script compilación completo
├── quick-build.ps1                # Script compilación rápida
├── setup-database.ps1             # Script configuración DB
├── BUILDING_WINDOWS.md            # Guía compilación detallada
├── DATABASE_SETUP.md              # Guía base de datos detallada
├── INICIO_RAPIDO_WINDOWS.md       # Inicio rápido
└── README_SETUP_ES.md             # Esta guía
```

### Bases de Datos

#### classicmangos (Mundo)
- **Tamaño:** ~500MB - 2GB (con classic-db)
- **Tablas principales:**
  - `creature_template` - NPCs
  - `gameobject_template` - Objetos del mundo
  - `item_template` - Items
  - `quest_template` - Quests
  - `spell_template` - Hechizos
- **Updates:** ~2000+ scripts SQL

#### classiccharacters (Personajes)
- **Tamaño:** Crece con uso
- **Tablas principales:**
  - `characters` - Datos de personajes
  - `character_inventory` - Inventarios
  - `guild` - Gremios
  - `playerbot_saved_data` - Datos bots (si habilitado)
- **Updates:** ~30+ scripts SQL

#### classicrealmd (Autenticación)
- **Tamaño:** Pequeña (~5MB)
- **Tablas principales:**
  - `account` - Cuentas de usuario
  - `realmlist` - Reinos disponibles
  - `account_banned` - Baneos
- **Updates:** ~5 scripts SQL

#### classiclogs (Logs)
- **Tamaño:** Crece con uso
- **Tablas principales:**
  - `logs` - Logs generales
  - `logs_anticheat` - Logs anticheat
- **Updates:** ~2 scripts SQL

---

## ❓ Preguntas Frecuentes

### ¿Por qué necesito Visual Studio?

Visual Studio 2022 proporciona el compilador MSVC necesario para compilar C++ en Windows. mangos-classic requiere C++20, que solo es soportado por MSVC v143 en Visual Studio 2022.

### ¿Qué son los Playerbots?

Los Playerbots son personajes de tu cuenta controlados por IA que te ayudan en el juego. Puedes tener hasta 9 bots simultáneamente.

**Comandos básicos de bots:**
- `.bot add NOMBRE_PERSONAJE` - Agregar un bot
- `.bot remove NOMBRE_PERSONAJE` - Remover un bot
- `.bot co follow` - Hacer que todos los bots te sigan
- `.bot co stay` - Hacer que todos los bots se queden quietos

### ¿Qué es el AHBot?

El Auction House Bot puebla automáticamente las casas de subastas con items, haciendo que el mundo se sienta más vivo en servidores con pocos jugadores.

### ¿Necesito descargar el cliente WoW?

Sí, necesitas una instalación válida del **cliente WoW 1.12.1** (Vanilla) para:
1. Extraer los datos (dbc, maps, vmaps, mmaps)
2. Conectarte al servidor

**Nota:** Solo el cliente original de Blizzard es soportado.

### ¿Cuánto espacio en disco necesito?

- **Código fuente:** ~500MB
- **Compilación:** ~2GB
- **Datos extraídos:** ~5GB
- **Base de datos:** ~1-3GB
- **Total recomendado:** 15GB libres

### ¿Cuánta RAM necesito?

- **Mínimo:** 4GB
- **Recomendado:** 8GB+
- **Óptimo:** 16GB

### ¿Puedo jugar solo con bots?

¡Sí! Con Playerbots habilitado puedes crear un grupo completo de bots y jugar solo. Los bots:
- Te siguen automáticamente
- Luchan a tu lado
- Recolectan loot
- Pueden usar habilidades de clase
- Responden a comandos básicos

### ¿Cómo actualizo el servidor?

```powershell
# 1. Actualizar código
git pull

# 2. Recompilar
.\quick-build.ps1 -Clean

# 3. Aplicar updates SQL
.\setup-database.ps1 -MySQLRootPassword "pass" -OnlyUpdates

# 4. Reiniciar servidores
```

### ¿Puedo ejecutar varios reinos?

Sí, pero necesitas:
1. Crear bases de datos separadas para cada reino
2. Configurar diferentes puertos
3. Agregar entradas en la tabla `realmlist`

### ¿Cómo creo cuentas GM?

```sql
-- Conectar a realmd
mysql -h localhost -u mangos -pmangos classicrealmd

-- gmlevel:
-- 0 = Jugador
-- 1 = Moderador
-- 2 = Game Master
-- 3 = Administrador

UPDATE account SET gmlevel = 3 WHERE username = 'nombre_usuario';
```

### ¿Los bots funcionan en dungeons?

Sí, los Playerbots pueden:
- Entrar a dungeons contigo
- Seguir tácticas básicas
- Usar habilidades apropiadas
- Recolectar y distribuir loot

Sin embargo, su IA no es perfecta en encuentros complejos.

### ¿Cómo desactivo Playerbots después de compilar?

No puedes desactivarlos sin recompilar. Debes recompilar sin el flag:

```powershell
.\build-windows.ps1 -EnablePlayerbots:$false
```

### Error: "Can't connect to MySQL server"

**Soluciones:**
1. Verificar que MySQL está corriendo:
   ```powershell
   Get-Service MySQL* | Start-Service
   ```

2. Verificar puerto en `my.ini`:
   ```ini
   [mysqld]
   port=3306
   ```

3. Verificar firewall no bloquea puerto 3306

### Error: "Access denied for user 'mangos'@'localhost'"

**Soluciones:**
1. Recrear usuario:
   ```powershell
   .\setup-database.ps1 -MySQLRootPassword "pass" -DropExisting
   ```

2. Verificar archivos `.conf` tienen credenciales correctas

### ¿Dónde están los logs?

- **Servidor de juego:** `build/bin/x64_Release/Server.log`
- **Servidor auth:** `build/bin/x64_Release/Realmd.log`
- **Logs SQL:** En base de datos `classiclogs`

---

## 📚 Documentación Adicional

### Archivos de Referencia

- **[BUILDING_WINDOWS.md](BUILDING_WINDOWS.md)** - Guía detallada de compilación
- **[DATABASE_SETUP.md](DATABASE_SETUP.md)** - Guía detallada de base de datos
- **[INICIO_RAPIDO_WINDOWS.md](INICIO_RAPIDO_WINDOWS.md)** - Referencia rápida
- **doc/PlayerBot/** - Documentación de Playerbots

### Recursos Externos

- **CMaNGOS Discord:** https://discord.gg/Dgzerzb
- **Wiki Oficial:** https://github.com/cmangos/issues/wiki
- **Classic-DB:** https://github.com/cmangos/classic-db
- **Playerbots GitHub:** https://github.com/cmangos/playerbots
- **Issues:** https://github.com/cmangos/issues/issues

---

## 🎉 ¡Disfruta!

Ahora tienes un servidor mangos-classic completamente funcional con soporte para Playerbots.

**¡Diviértete explorando Azeroth con tus bots!**

---

**Creado:** 2025-11-04
**Versión:** 1.0
**Mantenedor:** Tu Nombre
**Licencia:** GPL v2
