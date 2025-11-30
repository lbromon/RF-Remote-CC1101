# 📡 RF Remote Control Gateway (ESP32 + CC1101 + Telegram)

Este proyecto implementa un gateway IoT capaz de controlar dispositivos de radiofrecuencia (RF) que operan en **433MHz** y **868MHz** utilizando un **ESP32** y un módulo transceptor **CC1101**. Todo el sistema se controla de forma remota y segura a través de un **Bot de Telegram**.

## ✨ Características Principales

*   **Control Multi-Frecuencia**: Soporte para dispositivos de 433.92 MHz (Persianas, Ventiladores) y 868.33 MHz (Puertas de garaje/exterior).
*   **Interfaz Telegram Interactiva**: Menús intuitivos con botones (Inline Keyboards) para una fácil operación.
*   **Seguridad Avanzada**:
    *   **Whitelist**: Solo los usuarios autorizados (ID de chat) pueden interactuar con el bot.
    *   **Protección por PIN**: Las acciones sensibles (abrir puertas o persianas) requieren introducir un código PIN de seguridad.
*   **Gestión de Usuarios**: Sistema dinámico para añadir/eliminar usuarios autorizados y administradores.
*   **Configuración WiFi Sencilla**: Utiliza **WiFiManager**. Si no puede conectar a la red conocida, crea un Punto de Acceso (AP) para configurar las credenciales WiFi desde el móvil sin recompilar.
*   **Registro de Actividad (Logs)**: Guarda un historial de quién realizó qué acción y cuándo.
*   **Control de Ventiladores**: Protocolo específico implementado para controlar ventiladores de techo (Encendido, Luz, 6 Velocidades).

## 🛠️ Hardware Requerido

*   **Microcontrolador**: ESP32 (Probado en **Lolin C3 Mini**).
*   **Módulo RF**: CC1101 (Transceptor Sub-1GHz).
*   **Conexiones (Pinout por defecto para Lolin C3 Mini)**:

| CC1101 Pin | ESP32 Pin (GPIO) | Función |
| :--- | :--- | :--- |
| VCC | 3.3V | Alimentación |
| GND | GND | Tierra |
| SCK | 0 | SPI Clock |
| MISO | 1 | SPI MISO |
| MOSI | 21 | SPI MOSI |
| CSN (CS) | 3 | Chip Select |
| GDO0 | 4 | Interrupción/TX |
| GDO2 | 2 | Interrupción/RX |

## 📦 Dependencias de Software

El proyecto está desarrollado en **PlatformIO** (VS Code) y utiliza las siguientes librerías principales:

*   [RadioLib](https://github.com/jgromes/RadioLib) - Para el manejo del módulo CC1101.
*   [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) - Para la comunicación con Telegram.
*   [ArduinoJson](https://arduinojson.org/) - Para el manejo de datos JSON (teclados, logs, configuración).
*   [WiFiManager](https://github.com/tzapu/WiFiManager) - Para la gestión de la conexión WiFi.

## 🚀 Instalación y Configuración

1.  **Clonar el repositorio**:
    ```bash
    git clone https://github.com/LaboratorioGluon/ESP32_CC1101_MQTT.git
    ```
2.  **Abrir en PlatformIO**: Abre la carpeta del proyecto en Visual Studio Code con la extensión PlatformIO instalada.
3.  **Configurar Credenciales**:
    *   Navega a la carpeta `include/`.
    *   Renombra el archivo `secret_example.h` a `secret.h`.
    *   Edita `secret.h` con tus propios datos:
        ```cpp
        #define WIFI_SSID "TU_WIFI_SSID"       // Opcional, se puede configurar vía portal
        #define WIFI_PWD "TU_WIFI_PASSWORD"    // Opcional
        #define WIFIMANAGER_PWD "PASSWORD_AP"  // Contraseña para el AP de configuración
        #define TELEGRAM_BOT_TOKEN "TU_TOKEN"  // Token de tu Bot de Telegram
        #define SECURITY_PIN "1234"            // PIN para acciones seguras
        ```
    *   *Nota: `secret.h` está incluido en `.gitignore` para no subir tus claves al repositorio.*
4.  **Subir al ESP32**: Conecta tu ESP32 y dale al botón de "Upload" en PlatformIO.

## 📱 Uso

1.  **Primer Inicio**: Si no has configurado el WiFi en el código, el ESP32 creará una red WiFi llamada `RF_Remote_Config`. Conéctate a ella (usando la contraseña definida en `WIFIMANAGER_PWD`) y configura tu red WiFi doméstica.
2.  **Telegram**:
    *   Busca tu bot en Telegram e inicia el chat (`/start`).
    *   Si tu ID no está autorizado, deberás añadirlo manualmente en el código o usar un usuario administrador preconfigurado.
    *   **Menú Principal**: Muestra opciones para Persianas y Puertas.
    *   **Ventiladores**: Permite seleccionar qué ventiladores controlar y enviar comandos específicos.
    *   **Seguridad**: Al intentar abrir la puerta o persiana, el bot te pedirá el PIN.

## 📄 Licencia

Este proyecto es de código abierto. Siéntete libre de usarlo y modificarlo para tus necesidades.

---
*Desarrollado por LaboratorioGluon*