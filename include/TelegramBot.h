// TelegramBot.h
#ifndef TELEGRAM_BOT_H
#define TELEGRAM_BOT_H

#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

// Declaración externa del objeto bot y cliente seguro
extern WiFiClientSecure secured_client;
extern UniversalTelegramBot bot;

// Función principal para manejar el bot (llamada desde main loop)
void handleBot();

// Funciones de acción externas
extern void activarPersiana();
extern void activarPuerta(); // O activarPuertaExt si ese es el nombre final
extern void activarFan(int fanIndex, int commandIndex);

#endif // TELEGRAM_BOT_H