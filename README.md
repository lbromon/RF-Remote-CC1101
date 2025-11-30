# 📡 RF Remote Control Gateway

## 📖 Descripción del Proyecto
Este repositorio contiene el código fuente y la documentación de un **Gateway IoT Universal** diseñado para unificar el control de dispositivos de radiofrecuencia (433MHz y 868MHz) bajo una interfaz moderna y segura basada en **Telegram**.

El objetivo principal ha sido eliminar la dependencia de múltiples mandos físicos, centralizando el control de persianas, puertas de garaje y ventiladores de techo en un único dispositivo conectado a la nube.

## 🏗️ Arquitectura del Sistema
El sistema actúa como un puente bidireccional:
1.  **Interfaz de Usuario**: Un Bot de Telegram recibe comandos del usuario mediante menús interactivos.
2.  **Procesamiento**: Un ESP32 procesa la lógica, verifica permisos y gestiona la seguridad.
3.  **Transmisión RF**: Un módulo CC1101 emula las señales de los mandos originales para actuar sobre los dispositivos físicos.

## ✨ Funcionalidades Implementadas

### 1. Control Multi-Frecuencia
Gracias a la librería `RadioLib` y el módulo CC1101, el sistema es capaz de conmutar dinámicamente entre frecuencias:
*   **433.92 MHz**: Para el control de persianas motorizadas y ventiladores de techo.
*   **868.33 MHz**: Para sistemas de seguridad y puertas de garaje de largo alcance.

### 2. Seguridad y Control de Acceso
La seguridad ha sido una prioridad en el diseño:
*   **Whitelist Dinámica**: Solo los usuarios registrados en la memoria no volátil (Preferences) pueden interactuar con el sistema.
*   **Doble Factor (PIN)**: Las operaciones críticas (apertura de accesos) requieren la confirmación mediante un código PIN numérico.
*   **Logs de Auditoría**: El sistema registra cada acción realizada, guardando el usuario y la marca de tiempo para su posterior revisión.

### 3. Gestión de Conectividad
*   **WiFiManager**: Implementación de un portal cautivo para la configuración de credenciales WiFi sin necesidad de reprogramar el dispositivo. Si la conexión falla, el ESP32 crea su propio punto de acceso para ser reconfigurado.
*   **Reconexión Automática**: Sistema robusto de gestión de conexión para asegurar disponibilidad 24/7.

## 🔧 Especificaciones Técnicas

### Hardware
*   **MCU**: ESP32-C3 (Lolin C3 Mini)
*   **Transceptor**: Texas Instruments CC1101
*   **Protocolo**: OOK/ASK Modulation

### Stack de Software
*   **Framework**: Arduino sobre PlatformIO
*   **Comunicación**: API de Telegram (Long Polling)
*   **Persistencia**: ESP32 Preferences (NVS)
*   **Librerías Clave**: `RadioLib`, `UniversalTelegramBot`, `ArduinoJson`.

## 🚀 Despliegue

El proyecto está diseñado para ser compilado y subido mediante **PlatformIO**. La configuración sensible (Tokens, Claves WiFi iniciales) se gestiona a través de un archivo `secret.h` (no incluido en el repositorio por seguridad), permitiendo que el código base sea completamente agnóstico a las credenciales del usuario final.

---
*Proyecto desarrollado con fines educativos y de automatización del hogar.*