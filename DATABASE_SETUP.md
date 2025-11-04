# Guía de Configuración de Base de Datos

Esta guía te ayudará a configurar todas las bases de datos necesarias para ejecutar mangos-classic, incluyendo el soporte para Playerbots.

## 📋 Tabla de Contenidos

- [Requisitos Previos](#requisitos-previos)
- [Estructura de Bases de Datos](#estructura-de-bases-de-datos)
- [Instalación Rápida](#instalación-rápida)
- [Instalación Personalizada](#instalación-personalizada)
- [Descarga de Datos del Mundo](#descarga-de-datos-del-mundo)
- [Configuración de Archivos](#configuración-de-archivos)
- [Verificación](#verificación)
- [Resolución de Problemas](#resolución-de-problemas)

## 🔧 Requisitos Previos

### MySQL o MariaDB

Necesitas tener instalado **MySQL 8.0+** o **MariaDB 10.5+**.

#### Instalación en Windows 11

**Opción 1: MySQL (Recomendado)**
```powershell
choco install mysql -y
```

**Opción 2: MariaDB**
```powershell
choco install mariadb -y
```

Después de la instalación:
1. Asegúrate de que el servicio MySQL/MariaDB esté iniciado
2. Anota la contraseña root que configuraste durante la instalación

#### Verificar instalación

```powershell
mysql --version
```

## 🗄️ Estructura de Bases de Datos

mangos-classic usa **4 bases de datos**:

| Base de Datos | Nombre Por Defecto | Descripción |
|---------------|-------------------|-------------|
| **Mundo** | `classicmangos` | Datos del juego: NPCs, objetos, quests, spells, etc. |
| **Personajes** | `classiccharacters` | Datos de los jugadores: personajes, inventarios, progreso |
| **Autenticación** | `classicrealmd` | Cuentas de usuario y gestión de reinos |
| **Logs** | `classiclogs` | Registros de eventos del servidor |

### Usuario de Aplicación

- **Usuario:** `mangos` (configurable)
- **Contraseña:** `mangos` (configurable)
- **Host:** `localhost`

## 🚀 Instalación Rápida

### Paso 1: Preparar

Abre PowerShell como **Administrador** y navega al directorio del proyecto:

```powershell
cd C:\ruta\a\tu\mangos-classic
```

### Paso 2: Ejecutar el Script

```powershell
.\setup-database.ps1 -MySQLRootPassword "tu_password_root"
```

**¡Eso es todo!** El script:
1. ✅ Crea las 4 bases de datos
2. ✅ Crea el usuario `mangos` con permisos
3. ✅ Aplica todos los schemas base
4. ✅ Aplica todos los updates SQL
5. ✅ Configura soporte para Playerbots

## ⚙️ Instalación Personalizada

### Cambiar Nombres de Bases de Datos

```powershell
.\setup-database.ps1 `
    -MySQLRootPassword "tu_password" `
    -MangosDBName "mi_mangos" `
    -CharactersDBName "mi_characters" `
    -RealmdDBName "mi_realmd" `
    -LogsDBName "mi_logs"
```

### Cambiar Usuario y Contraseña de Mangos

```powershell
.\setup-database.ps1 `
    -MySQLRootPassword "tu_password" `
    -MangosUser "mi_usuario" `
    -MangosPassword "mi_password_segura"
```

### Servidor MySQL Remoto

```powershell
.\setup-database.ps1 `
    -MySQLRootPassword "tu_password" `
    -MySQLHost "192.168.1.100" `
    -MySQLPort 3306
```

### Reinstalar (Eliminar bases de datos existentes)

```powershell
.\setup-database.ps1 `
    -MySQLRootPassword "tu_password" `
    -DropExisting
```

### Solo Aplicar Updates (sin recrear schemas)

```powershell
.\setup-database.ps1 `
    -MySQLRootPassword "tu_password" `
    -OnlyUpdates
```

### Saltar Updates

```powershell
.\setup-database.ps1 `
    -MySQLRootPassword "tu_password" `
    -SkipUpdates
```

## 🌍 Descarga de Datos del Mundo

Después de ejecutar el script, **debes descargar e importar la base de datos del mundo**.

### Opción 1: Classic-DB (Recomendado para Vanilla)

1. **Descargar:**
   ```powershell
   git clone https://github.com/cmangos/classic-db.git
   cd classic-db
   ```

2. **Configurar InstallFullDB.config:**

   Edita el archivo `InstallFullDB.config` con tus datos:
   ```
   MYSQL_HOST="localhost"
   MYSQL_PORT="3306"
   MYSQL_USERNAME="root"
   MYSQL_PASSWORD="tu_password_root"
   MYSQL_DATABASE="classicmangos"
   CORE_PATH="C:/ruta/a/mangos-classic"
   ```

3. **Ejecutar instalador:**

   En Linux/Git Bash:
   ```bash
   ./InstallFullDB.sh
   ```

   En Windows PowerShell:
   ```powershell
   bash InstallFullDB.sh
   ```

### Opción 2: Importación Manual

Si tienes un archivo SQL de la base de datos del mundo:

```powershell
mysql -h localhost -u root -p classicmangos < ruta/al/world_database.sql
```

## 📝 Configuración de Archivos

Después de configurar la base de datos, **debes actualizar los archivos de configuración** del servidor.

### 1. Archivo mangosd.conf

Ubicación: `build/bin/x64_Release/mangosd.conf` (o donde compilaste)

```ini
###################################################################################################################
# CONEXIONES DE BASE DE DATOS
###################################################################################################################

# Mundo
WorldDatabaseInfo = "localhost;3306;mangos;mangos;classicmangos"

# Personajes
CharacterDatabaseInfo = "localhost;3306;mangos;mangos;classiccharacters"

# Logs (opcional)
LogsDatabaseInfo = "localhost;3306;mangos;mangos;classiclogs"
```

### 2. Archivo realmd.conf

Ubicación: `build/bin/x64_Release/realmd.conf`

```ini
###################################################################################################################
# CONEXIÓN DE BASE DE DATOS
###################################################################################################################

LoginDatabaseInfo = "localhost;3306;mangos;mangos;classicrealmd"
```

### 3. Configuración de Playerbots

Si compilaste con `-EnablePlayerbots`, también necesitas configurar:

Ubicación: `build/bin/x64_Release/playerbot.conf`

```ini
###################################################################################################################
# PLAYERBOTS CONFIGURACIÓN
###################################################################################################################

# Número máximo de bots por jugador
PlayerbotAI.MaxNumBots = 9

# Habilitar debug en whispers
PlayerbotAI.DebugWhisper = 0

# Distancia de seguimiento
PlayerbotAI.FollowDistanceMin = 0.5
PlayerbotAI.FollowDistanceMax = 1.0
```

## ✅ Verificación

### Verificar que las bases de datos fueron creadas

```powershell
mysql -h localhost -u root -p -e "SHOW DATABASES;"
```

Deberías ver:
- `classicmangos`
- `classiccharacters`
- `classicrealmd`
- `classiclogs`

### Verificar usuario mangos

```powershell
mysql -h localhost -u mangos -pmangos -e "SELECT 1;"
```

Si funciona, el usuario está configurado correctamente.

### Verificar tablas en la base de datos de personajes

```powershell
mysql -h localhost -u mangos -pmangos classiccharacters -e "SHOW TABLES LIKE 'playerbot%';"
```

Deberías ver:
- `playerbot_saved_data`

### Verificar que los datos del mundo fueron importados

```powershell
mysql -h localhost -u mangos -pmangos classicmangos -e "SELECT COUNT(*) as creature_count FROM creature_template;"
```

Debería mostrar miles de criaturas (si importaste la base de datos del mundo).

## 🔍 Resolución de Problemas

### Error: "Access denied for user"

**Problema:** No puedes conectar a MySQL.

**Solución:**
```powershell
# Verificar que MySQL está corriendo
Get-Service MySQL* | Select-Object Name, Status

# Iniciar servicio si está detenido
Start-Service MySQL
```

### Error: "Unknown database"

**Problema:** Las bases de datos no existen.

**Solución:**
Ejecuta el script de nuevo sin `-OnlyUpdates`:
```powershell
.\setup-database.ps1 -MySQLRootPassword "tu_password"
```

### Error: "Can't connect to MySQL server"

**Problema:** MySQL no está escuchando en el puerto correcto.

**Solución:**
Verifica el puerto en `my.ini` (MySQL) o `my.cnf` (MariaDB):
```ini
[mysqld]
port=3306
```

### Error: "playerbot_saved_data table doesn't exist"

**Problema:** El update de playerbots no se aplicó.

**Solución:**
Aplica manualmente:
```powershell
mysql -h localhost -u root -p classiccharacters < sql/updates/characters/z2698_01_characters_playerbot_saved_data.sql
```

### Error: "Table already exists"

**Problema:** Intentas crear tablas que ya existen.

**Solución:**
Usa la opción `-OnlyUpdates` para solo aplicar updates:
```powershell
.\setup-database.ps1 -MySQLRootPassword "tu_password" -OnlyUpdates
```

O elimina las bases de datos primero:
```powershell
.\setup-database.ps1 -MySQLRootPassword "tu_password" -DropExisting
```

### La base de datos del mundo está vacía

**Problema:** No importaste classic-db.

**Solución:**
Sigue los pasos en [Descarga de Datos del Mundo](#descarga-de-datos-del-mundo).

### Los comandos de Playerbots no funcionan

**Problema:** No compilaste con soporte para Playerbots.

**Solución:**
Recompila con:
```powershell
.\build-windows.ps1 -EnablePlayerbots
```

## 📊 Estructura Detallada de las Bases de Datos

### classicmangos (Mundo)

Contiene todos los datos del juego:
- `creature_template` - Plantillas de NPCs
- `gameobject_template` - Plantillas de objetos del juego
- `item_template` - Plantillas de ítems
- `quest_template` - Quests disponibles
- `spell_template` - Hechizos y habilidades
- Y muchas más...

### classiccharacters (Personajes)

Contiene datos de jugadores:
- `characters` - Información de personajes
- `character_inventory` - Inventarios
- `guild` - Gremios
- `playerbot_saved_data` - Datos de bots (si Playerbots está habilitado)
- Y más...

### classicrealmd (Autenticación)

Contiene autenticación:
- `account` - Cuentas de usuario
- `realmlist` - Lista de reinos disponibles
- `account_banned` - Baneos

### classiclogs (Logs)

Contiene registros:
- `logs` - Logs generales
- `logs_anticheat` - Logs de anticheat
- Y más...

## 🔐 Seguridad

### Cambiar contraseña por defecto

**¡IMPORTANTE!** La contraseña por defecto `mangos` **NO** es segura para producción.

Cambia la contraseña:

```sql
ALTER USER 'mangos'@'localhost' IDENTIFIED BY 'nueva_password_segura';
FLUSH PRIVILEGES;
```

Luego actualiza los archivos `.conf` con la nueva contraseña.

### Acceso remoto

Si necesitas acceso remoto (no recomendado para producción):

```sql
CREATE USER 'mangos'@'%' IDENTIFIED BY 'password_segura';
GRANT ALL PRIVILEGES ON classicmangos.* TO 'mangos'@'%';
GRANT ALL PRIVILEGES ON classiccharacters.* TO 'mangos'@'%';
GRANT ALL PRIVILEGES ON classicrealmd.* TO 'mangos'@'%';
GRANT ALL PRIVILEGES ON classiclogs.* TO 'mangos'@'%';
FLUSH PRIVILEGES;
```

## 📚 Recursos Adicionales

- **Classic-DB GitHub:** https://github.com/cmangos/classic-db
- **CMaNGOS Wiki:** https://github.com/cmangos/issues/wiki
- **Discord de CMaNGOS:** https://discord.gg/Dgzerzb
- **Documentación SQL:** https://github.com/cmangos/mangos-classic/tree/master/sql

## 🆘 ¿Necesitas Ayuda?

Si encuentras problemas:

1. Revisa la sección [Resolución de Problemas](#resolución-de-problemas)
2. Verifica los logs de MySQL (generalmente en `C:\ProgramData\MySQL\MySQL Server 8.0\Data\*.err`)
3. Consulta la [Wiki oficial](https://github.com/cmangos/issues/wiki)
4. Pregunta en el [Discord de CMaNGOS](https://discord.gg/Dgzerzb)

---

**Última actualización:** 2025-11-04
