#include "wifi_local.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "secret.h"
#include <Preferences.h>

WiFiClientSecure secured_client;

// Variables para guardar parámetros custom
char telegram_token[60] = TELEGRAM_BOT_TOKEN;
char admin_id[20] = "399392546";

// Flag para saber si se debe guardar config
bool shouldSaveConfig = false;

// Callback para notificar que hay que guardar
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void wifi_init()
{
    WiFiManager wifiManager;
    
    // 1. Leer parámetros guardados en Preferences (si existen)
    Preferences prefs;
    prefs.begin("bot_config", true); // Read-only
    String saved_token = prefs.getString("tg_token", "");
    String saved_admin = prefs.getString("admin_id", "");
    prefs.end();

    if (saved_token != "") saved_token.toCharArray(telegram_token, 60);
    if (saved_admin != "") saved_admin.toCharArray(admin_id, 20);

    // 2. Configurar parámetros custom para WiFiManager
    WiFiManagerParameter custom_tg_token("tg_token", "Telegram Bot Token", telegram_token, 60);
    WiFiManagerParameter custom_admin_id("admin_id", "Admin ID", admin_id, 20);

    wifiManager.addParameter(&custom_tg_token);
    wifiManager.addParameter(&custom_admin_id);
    
    // Callback para guardar
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    // 3. Intentar conectar o crear AP
    
    // Configuración de Timeouts
    wifiManager.setConnectTimeout(30); // Intentar conectar al WiFi guardado durante 30s antes de abrir el AP
    wifiManager.setTimeout(180);       // Si se abre el AP, mantenerlo activo 180s (3 min) antes de salir/reiniciar
    
    if (!wifiManager.autoConnect("RF_Remote_Config", WIFIMANAGER_PWD)) {
        Serial.println("Fallo al conectar y timeout alcanzado");
        ESP.restart();
        delay(1000);
    }

    // 4. Si llegamos aquí, estamos conectados
    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // 5. Guardar parámetros si han cambiado
    if (shouldSaveConfig) {
        Serial.println("Guardando configuración personalizada...");
        strcpy(telegram_token, custom_tg_token.getValue());
        strcpy(admin_id, custom_admin_id.getValue());

        Preferences p;
        p.begin("bot_config", false); // Read/Write
        p.putString("tg_token", telegram_token);
        p.putString("admin_id", admin_id);
        p.end();
    }

    // 6. Configurar cliente seguro
    secured_client.setInsecure(); // Usar setInsecure para evitar problemas con certificados caducados

    // 7. Configurar Hora (NTP) para Logs
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Hora España (Madrid)
    tzset();
}

// Getter para que otros módulos accedan al token dinámico
String getTelegramToken() {
    // Leer de memoria si es posible, o usar la variable global actualizada
    return String(telegram_token);
}


