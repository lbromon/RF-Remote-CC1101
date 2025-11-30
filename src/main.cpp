// main.cpp
#include <Arduino.h>
#include "wifi_local.h"
#include "radiotx.h"   
#include "fanControl.h" 
#include "TelegramBot.h" 
#include "UserManager.h" // Incluir gestor de usuarios
#include "LogManager.h"  // Incluir gestor de logs

// --- Instancias Globales ---
// El bot y secured_client se definen/declaran en sus archivos

void setup() {
    Serial.begin(115200);
    Serial.println("Puerto serie inicializado");

    // Inicializar Gestor de Usuarios (Preferences)
    userManager.begin();
    
    // Inicializar Gestor de Logs
    logManager.begin();

    // Inicializar WiFi (y configura secured_client)
    wifi_init();

    // Re-inicializar el bot con el token correcto (si cambió en WiFiManager)
    // Nota: UniversalTelegramBot permite updateToken()
    bot.updateToken(getTelegramToken());

    // Eliminar menú de comandos (burguer menu)
    bot.setMyCommands(""); 

    // Init radio
    radio_init();

    // Configurar Long Polling para el bot (menor consumo, respuesta más rápida)
    bot.longPoll = 5; // Esperar hasta 5 segundos por mensaje
    bot.waitForResponse = 10000; // Timeout de socket > longPoll

    Serial.println("Setup completo.");
}

void loop() {
  // Gestionar bot de Telegram
  handleBot();

  // Pequeño delay para estabilidad general (reducido)
  delay(1); // Usar 1ms o incluso 0 si no causa problemas con WiFi/otras tareas
}