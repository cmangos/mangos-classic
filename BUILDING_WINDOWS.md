# Guía de Compilación para Windows 11

Esta guía te ayudará a compilar mangos-classic en Windows 11 usando un script automatizado.

## 📋 Tabla de Contenidos

- [Requisitos Previos](#requisitos-previos)
- [Instalación Rápida (Automatizada)](#instalación-rápida-automatizada)
- [Instalación Manual (Si el script falla)](#instalación-manual-si-el-script-falla)
- [Uso del Script](#uso-del-script)
- [Resolución de Problemas](#resolución-de-problemas)
- [Configuración Avanzada](#configuración-avanzada)

## 🔧 Requisitos Previos

### Instalación Manual Requerida

**Visual Studio 2022** debe instalarse manualmente antes de ejecutar el script:

1. **Descargar Visual Studio 2022**
   - Descarga la Community Edition (gratuita): https://visualstudio.microsoft.com/es/downloads/
   - También puedes usar Professional o Enterprise si los tienes

2. **Durante la instalación, selecciona:**
   - ✅ **"Desarrollo para el escritorio con C++"** (Desktop development with C++)
   - ✅ **MSVC v143 - VS 2022 C++ x64/x86 build tools (latest)**
   - ✅ **Windows 11 SDK** (o Windows 10 SDK)
   - ✅ **CMake tools for Windows** (opcional, pero recomendado)

3. **Componentes individuales adicionales (opcional pero recomendado):**
   - C++ core features
   - C++ 2022 Redistributable Update
   - C++ CMake tools for Windows

### Instalación Automática

El script instalará automáticamente:
- ✅ Chocolatey (gestor de paquetes para Windows)
- ✅ Git
- ✅ CMake 3.12+
- ✅ Boost (boost-msvc-14.3)

### Librerías Incluidas en el Repositorio

Estas ya están incluidas en el proyecto y **NO** necesitan instalación:
- MySQL Client Libraries
- OpenSSL 3.x Libraries

## 🚀 Instalación Rápida (Automatizada)

### Paso 1: Preparar PowerShell

Abre PowerShell como **Administrador**:
- Presiona `Win + X`
- Selecciona "Windows Terminal (Administrador)" o "PowerShell (Administrador)"

### Paso 2: Navegar al directorio del proyecto

```powershell
cd C:\ruta\a\tu\mangos-classic
```

### Paso 3: Permitir ejecución de scripts (si es necesario)

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Paso 4: Ejecutar el script

```powershell
.\build-windows.ps1
```

Eso es todo! El script:
1. Verificará todas las dependencias
2. Instalará automáticamente lo que falte (excepto Visual Studio)
3. Configurará el proyecto con CMake
4. Compilará el proyecto
5. Mostrará la ubicación de los binarios generados

## 📝 Uso del Script

### Compilación básica (Release)

```powershell
.\build-windows.ps1
```

### Compilación Debug

```powershell
.\build-windows.ps1 -BuildType Debug
```

### Compilación con información de depuración

```powershell
.\build-windows.ps1 -BuildType RelWithDebInfo
```

### Compilación optimizada para tamaño

```powershell
.\build-windows.ps1 -BuildType MinSizeRel
```

### Omitir verificación de dependencias (si ya están instaladas)

```powershell
.\build-windows.ps1 -SkipDependencyCheck
```

### Especificar directorio de compilación personalizado

```powershell
.\build-windows.ps1 -BuildDir "mi-build"
```

### Combinar opciones

```powershell
.\build-windows.ps1 -BuildType Debug -BuildDir "build-debug" -SkipDependencyCheck
```

## 🔨 Instalación Manual (Si el script falla)

Si el script automático no funciona, puedes instalar las dependencias manualmente:

### 1. Instalar Chocolatey

Abre PowerShell como Administrador y ejecuta:

```powershell
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
```

Cierra y reabre PowerShell como Administrador.

### 2. Instalar Git

```powershell
choco install git -y
```

### 3. Instalar CMake

```powershell
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System' -y
```

### 4. Instalar Visual Studio 2022

Sigue las instrucciones en [Requisitos Previos](#instalación-manual-requerida)

### 5. Instalar Boost

```powershell
choco install boost-msvc-14.3 -y
```

Después de la instalación, configura la variable de entorno BOOST_ROOT:

```powershell
$boostPath = (Get-ChildItem -Path "C:\local\" -Filter "boost_*" -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
[System.Environment]::SetEnvironmentVariable("BOOST_ROOT", $boostPath, "Machine")
```

### 6. Compilar manualmente

```powershell
# Crear directorio de compilación
mkdir build
cd build

# Configurar con CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Compilar (usando todos los núcleos)
cmake --build . --config Release --parallel
```

## 🔍 Resolución de Problemas

### Error: "No se puede ejecutar scripts en este sistema"

**Solución:**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Error: "Visual Studio 2022 no encontrado"

**Solución:**
- Instala Visual Studio 2022 manualmente con el workload "Desktop development with C++"
- Verifica la instalación ejecutando:
  ```powershell
  & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -version "[17.0,18.0)"
  ```

### Error: "BOOST_ROOT no está configurado"

**Solución:**
```powershell
# Buscar instalación de Boost
$boostPath = (Get-ChildItem -Path "C:\local\" -Filter "boost_*" -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
# Configurar variable de entorno
[System.Environment]::SetEnvironmentVariable("BOOST_ROOT", $boostPath, "Machine")
# Reiniciar PowerShell
```

### Error: "CMake version too old"

**Solución:**
```powershell
choco upgrade cmake -y
```

### Error: "Git no encontrado" después de instalar

**Solución:**
- Cierra y reabre PowerShell
- O actualiza las variables de entorno:
  ```powershell
  $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
  ```

### Error durante la compilación: "No se encuentra vcvarsall.bat"

**Solución:**
- Asegúrate de tener instalado el componente "MSVC v143" en Visual Studio
- Reinstala Visual Studio con los componentes correctos

### El script se queda colgado durante la instalación de Boost

**Solución:**
- Boost es grande y puede tardar 10-20 minutos en instalarse
- Si realmente está colgado, presiona Ctrl+C y prueba:
  ```powershell
  choco install boost-msvc-14.3 -y --force
  ```

## ⚙️ Configuración Avanzada

### Compilar solo componentes específicos

Puedes editar el CMakeLists.txt o usar opciones de CMake:

```powershell
cmake .. -DBUILD_EXTRACTORS=OFF -DBUILD_PLAYERBOTS=ON
```

### Usar un compilador específico

```powershell
cmake .. -G "Visual Studio 17 2022" -A x64
```

### Compilación en paralelo con límite de núcleos

```powershell
cmake --build . --config Release --parallel 4
```

### Ver todas las opciones de compilación

```powershell
cmake -LAH ..
```

## 📁 Estructura de Archivos Generados

Después de una compilación exitosa, encontrarás:

```
build/
└── bin/
    └── x64_Release/          # (o Debug, RelWithDebInfo, etc.)
        ├── mangosd.exe       # Servidor de juego
        ├── realmd.exe        # Servidor de autenticación
        ├── *.dll             # Librerías necesarias
        └── ...               # Otros ejecutables y archivos
```

## 📚 Recursos Adicionales

- **Repositorio oficial:** https://github.com/cmangos/mangos-classic
- **Wiki del proyecto:** https://github.com/cmangos/issues/wiki
- **Discord de CMaNGOS:** https://discord.gg/Dgzerzb
- **Issues/Problemas:** https://github.com/cmangos/issues/issues

## 📄 Licencia

Este proyecto usa la licencia GPL v2. Ver archivo [COPYING](COPYING) para más detalles.

---

## 🆘 ¿Necesitas ayuda?

Si encuentras problemas:

1. Lee la sección [Resolución de Problemas](#resolución-de-problemas)
2. Revisa los logs de error en PowerShell
3. Consulta la [Wiki oficial](https://github.com/cmangos/issues/wiki)
4. Pregunta en el [Discord de CMaNGOS](https://discord.gg/Dgzerzb)
5. Abre un issue en [GitHub](https://github.com/cmangos/issues/issues)

---

**Última actualización:** 2025-11-04
