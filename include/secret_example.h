// secret_example.h
// Plantilla de ejemplo para el archivo secret.h
#ifndef __SECRET_H__
#define __SECRET_H__

// --- Credenciales WiFi ---
#define WIFI_SSID "TU_SSID"
#define WIFI_PWD "TU_PASSWORD"

// --- Password AP WiFiManager ---
#define WIFIMANAGER_PWD "PASSWORD_PARA_AP"

// --- Token del Bot ---
#define TELEGRAM_BOT_TOKEN "TU_TELEGRAM_BOT_TOKEN" 

#define SECURITY_PIN "1234"

// --- Whitelist de Usuarios ---
extern const char* ALLOWED_CHAT_IDS[];
extern const int ALLOWED_CHAT_IDS_COUNT;

#endif // __SECRET_H__
