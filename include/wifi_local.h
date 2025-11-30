#ifndef __WIFI_LOCAL_H__
#define __WIFI_LOCAL_H__

#include <WiFi.h>
#include <WiFiClientSecure.h>

void wifi_init();
String getTelegramToken(); // Nueva función para obtener el token dinámico

// Cambia la definición por una declaración externa
extern WiFiClientSecure secured_client;

#endif //__WIFI_LOCAL_H__