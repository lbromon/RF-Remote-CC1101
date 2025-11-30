// fanControl.cpp
#include "fanControl.h"
#include "radiotx.h" // Incluye la declaración de radio_send_fan, etc.
#include <Arduino.h>

// --- Datos Persiana ---
uint8_t persianaData[37] = {
1,0,2,0,2,3,1,3,1,0,2,0,2,0,2,3,1,0,2,0,2,0,2,0,2,0,2,0,2,3,1,3,1,0,2,0,2
};

// --- Datos Puerta ---
uint8_t puertaData[132] = {
2,0,1,3,2,0,1,3,1,3,2,0,2,0,1,3,2,0,2,0,2,0,1,3,1,3,2,0,1,3,1,3,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,2,0,1,3,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,1,3,2,0,2,0,1,3,2,0,2,0,1,3,2,0,2,0,2,0,2
};

// --- Datos Ventiladores ---
const int FAN_COMMAND_LEN = 49;
const int NUM_FAN_COMMANDS = 8; // Power, Light, Speed1-6
const int MAX_FANS_DATA = 4;

uint8_t fanData[MAX_FANS_DATA][NUM_FAN_COMMANDS][FAN_COMMAND_LEN] =
{
    // Datos para Ventilador 1 (Índice 0)
    {
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,1,3,1,3,2,0,1}, //POWER-1 [0][0]
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,1,3,2,0,1,3,1}, //LIGHT-1 [0][1]
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,1,3,2,0,2,0,1}, //SPEED1-1 [0][2]
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,2,0,1,3,1,3,1}, //SPEED2-1 [0][3]
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,2,0,1,3,2,0,1}, //SPEED3-1 [0][4]
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,2,0,2,0,1,3,1}, //SPEED4-1 [0][5]
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,1,3,2,0,2,0,2,0,1}, //SPEED5-1 [0][6]
        {1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,2,0,2,0,1,3,2,0,2,0,1,3,1,3,1,3,1,3,2,0,1,3,1,3,1,3,1}  //SPEED6-1 [0][7]
    },
    // Datos para Ventilador 2 (Índice 1)
    { {0} },
    // Datos para Ventilador 3 (Índice 2)
    { {0} },
    // Datos para Ventilador 4 (Índice 3)
    { {0} }
};


// --- Funciones de Activación ---

void activarPersiana()
{
    Serial.println("Activando Persiana...");
    // Llama a la función específica de envío para la persiana
    radio_send_persiana(persianaData, sizeof(persianaData));
}

void activarPuerta()
{
    Serial.println("Activando Puerta Exterior...");
    // Llama a la función específica de envío para la puerta
    radio_send_puerta(puertaData, sizeof(puertaData));
}

void activarFan(int fanIndex, int commandIndex)
{
    // Validación de índices
    if (fanIndex < 0 || fanIndex >= MAX_FANS_DATA) {
        Serial.print("Error: Índice de ventilador inválido: "); Serial.println(fanIndex);
        return;
    }
    if (commandIndex < 0 || commandIndex >= NUM_FAN_COMMANDS) {
        Serial.print("Error: Índice de comando inválido: "); Serial.println(commandIndex);
        return;
    }

    Serial.print("Activando Ventilador "); Serial.print(fanIndex + 1);
    Serial.print(" con Comando "); Serial.println(commandIndex);

    // --- Llama a la función de envío específica para el ventilador ---
    radio_send_fan(fanData[fanIndex][commandIndex], FAN_COMMAND_LEN);
}