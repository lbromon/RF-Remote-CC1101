#include "radiotx.h"

#include <RadioLib.h>

constexpr float RADIO_FREQ_433 = 433.92f;
constexpr float RADIO_FREQ_868 = 868.33f;
//ESP32-C3
constexpr uint8_t RADIO_SCK  = 0;
constexpr uint8_t RADIO_MISO = 1;
constexpr uint8_t RADIO_MOSI = 21;
constexpr uint8_t CS   = 3;
constexpr uint8_t GDO0 = 4;
constexpr uint8_t GDO2 = 2;

CC1101 radio = new Module(CS, GDO0, RADIOLIB_NC, GDO2);

void radio_init()
{
    SPI.begin(RADIO_SCK, RADIO_MISO, RADIO_MOSI); 
    int state = radio.begin(RADIO_FREQ_433);
    if ( state == RADIOLIB_ERR_NONE )
    {
        Serial.println("Radio iniciada");
    }
    else
    {
        Serial.print("Radio fallida:");
        Serial.print(state);
        while(1);
    }

    radio.setOOK(true);
    radio.setBitRate(10.0f);
    radio.setOutputPower(10);
}

inline void pulse(uint32_t time)
{
    digitalWrite(GDO0, HIGH);
    delayMicroseconds(time);
    digitalWrite(GDO0, LOW);
}

void radio_send_persiana(uint8_t* data, uint8_t len)
{
    Serial.println("Iniciando transmisión persiana (433 MHz)...");
    pinMode(GDO0, OUTPUT); // Configurar GDO0 para salida (transmisión directa)

    // Entrar en modo de transmisión directa
    int state = radio.transmitDirect();
    if ( state != RADIOLIB_ERR_NONE ) {
        Serial.print("Error al iniciar TX directo (Persiana): "); Serial.println(state);
        return; // Salir si no se puede entrar en modo TX
    }

    // Asegurar GDO2 bajo si afecta la transmisión (depende de la configuración del CC1101)
    digitalWrite(GDO2, LOW);
    delay(10); // Pequeña pausa

    // Bucle de repetición de la trama
    for (uint8_t j = 0; j < 10; j++){
        // Bucle de envío de datos (pulsos)
        for (uint8_t i = 0; i < len; i++) {
            switch (data[i]) {
                case 0: delayMicroseconds(250); break;
                case 1: pulse(200); break;
                case 2: pulse(600); break;
                case 3: delayMicroseconds(750); break;
            }
        }
        delay(10); // Pausa entre repeticiones
    }

    // Finalizar transmisión y salir del modo TX
    radio.finishTransmit();
    Serial.println("Transmisión finalizada (Persiana).");
}

// --- Envío Puerta (868 MHz) ---
void radio_send_puerta(uint8_t* data, uint8_t len)
{
    int state;

    // 1. Cambiar a 868 MHz
    Serial.println("Configurando radio a " + String(RADIO_FREQ_868) + " MHz para puerta...");
    state = radio.setFrequency(RADIO_FREQ_868);
    radio.setOutputPower(10);
    if ( state != RADIOLIB_ERR_NONE ) {
        Serial.print("Error al configurar frecuencia 868: "); Serial.println(state);
        return;
    }

    // 2. Preparar transmisión
    Serial.println("Iniciando transmisión puerta (868 MHz)...");
    pinMode(GDO0, OUTPUT); // Configurar GDO0 para salida

    // 3. Entrar en modo de transmisión directa
    state = radio.transmitDirect();
    if ( state != RADIOLIB_ERR_NONE ) {
        Serial.print("Error al iniciar TX directo (Puerta): "); Serial.println(state);
        Serial.println("Intentando restaurar radio a " + String(RADIO_FREQ_433) + " MHz...");
        int restore_state = radio.setFrequency(RADIO_FREQ_433);
        if ( restore_state != RADIOLIB_ERR_NONE ) {
            Serial.print("Error al restaurar frecuencia 433 después de fallo TX: "); Serial.println(restore_state);
        }
        return;
    }

    digitalWrite(GDO2, LOW); // Asegurar GDO2 bajo si es necesario
    delay(10);

    // 4. Secuencia de pulsos específica para la puerta
    pulse(10000);               // Pulso inicial largo
    delay(45);                  // Pausa (ajustar si es necesario)

    // Bucle de repetición de la trama de datos
    for (uint8_t j = 0; j < 10; j++){

        for (uint8_t p = 0; p < 11; p++) { 
        pulse(225);
        delayMicroseconds(225);
        }
        pulse(225);
        delayMicroseconds(2200);

        for (uint8_t i = 0; i < len; i++) {
            switch (data[i]) {
                case 0: delayMicroseconds(225); break;
                case 1: pulse(225); break;
                case 2: pulse(450); break;
                case 3: delayMicroseconds(450); break;
            }
        }
        delay(10); // Pausa entre repeticiones
    }

    // 5. Finalizar transmisión
    radio.finishTransmit();
    Serial.println("Transmisión finalizada (Puerta).");

    // 6. Restaurar a 433 MHz
    Serial.println("Restaurando radio a " + String(RADIO_FREQ_433) + " MHz...");
    state = radio.setFrequency(RADIO_FREQ_433);
    if ( state != RADIOLIB_ERR_NONE ) {
        Serial.print("Error al restaurar frecuencia 433: "); Serial.println(state);
    }
}

void radio_send_fan(uint8_t* data, uint8_t len)
{
    Serial.println("Iniciando transmisión fan (433 MHz)...");
    pinMode(GDO0, OUTPUT); // Configurar GDO0 para salida (transmisión directa)

    // Entrar en modo de transmisión directa
    int state = radio.transmitDirect();
    if ( state != RADIOLIB_ERR_NONE ) {
        Serial.print("Error al iniciar TX directo (Fan): "); Serial.println(state);
        return; // Salir si no se puede entrar en modo TX
    }

    // Asegurar GDO2 bajo si afecta la transmisión (depende de la configuración del CC1101)
    digitalWrite(GDO2, LOW);
    delay(10); // Pequeña pausa

    // Bucle de repetición de la trama
    for (uint8_t j = 0; j < 10; j++){
        // Bucle de envío de datos (pulsos)
        for (uint8_t i = 0; i < len; i++) {
            switch (data[i]) {
                case 0: delayMicroseconds(260); break;
                case 1: pulse(260); break;
                case 2: pulse(770); break;
                case 3: delayMicroseconds(770); break;
            }
        }
        delay(10); // Pausa entre repeticiones
    }

    // Finalizar transmisión y salir del modo TX
    radio.finishTransmit();
    Serial.println("Transmisión finalizada (Fan).");
}
