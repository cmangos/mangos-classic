# 🚀 Inicio Rápido - Compilación en Windows 11

## ⚡ Instalación Rápida (3 pasos)

### 1️⃣ Instalar Visual Studio 2022

**¡IMPORTANTE!** Esto debe hacerse manualmente primero:

1. Descargar: https://visualstudio.microsoft.com/es/downloads/ (Community Edition es gratuita)
2. Durante la instalación, seleccionar:
   - ✅ **"Desarrollo para el escritorio con C++"**
   - ✅ **MSVC v143 build tools**
   - ✅ **Windows 11 SDK**

### 2️⃣ Ejecutar el script automatizado

Abre **PowerShell como Administrador** y ejecuta:

```powershell
cd C:\ruta\a\tu\mangos-classic
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
.\build-windows.ps1
```

### 3️⃣ ¡Listo!

El script instalará automáticamente:
- Chocolatey
- Git
- CMake
- Boost

Y compilará el proyecto. Los binarios estarán en `build\bin\x64_Release\`

---

## 📝 Otros Comandos Útiles

### Compilación Debug
```powershell
.\build-windows.ps1 -BuildType Debug
```

### Compilación rápida (si ya tienes todo instalado)
```powershell
.\quick-build.ps1
```

### Limpiar y recompilar
```powershell
.\quick-build.ps1 -Clean
```

---

## ❓ ¿Problemas?

### "No se puede ejecutar scripts"
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### "Visual Studio no encontrado"
- Instala Visual Studio 2022 con "Desktop development with C++"

### "BOOST_ROOT no configurado"
```powershell
$boostPath = (Get-ChildItem "C:\local\" -Filter "boost_*" -Directory | Sort-Object Name -Descending | Select-Object -First 1).FullName
[System.Environment]::SetEnvironmentVariable("BOOST_ROOT", $boostPath, "Machine")
```

### Más ayuda
Ver **[BUILDING_WINDOWS.md](BUILDING_WINDOWS.md)** para la guía completa.

---

## 📂 Archivos del Proyecto

- `build-windows.ps1` - Script completo con instalación de dependencias
- `quick-build.ps1` - Script rápido para recompilaciones
- `BUILDING_WINDOWS.md` - Guía detallada completa
- `INICIO_RAPIDO_WINDOWS.md` - Este archivo (referencia rápida)

---

**¿Primera vez?** → Usa `build-windows.ps1`
**Ya tienes todo instalado?** → Usa `quick-build.ps1`
