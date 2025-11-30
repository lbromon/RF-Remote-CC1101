// TelegramBot.cpp
#include "TelegramBot.h"
#include "fanControl.h" // Para activarFan, activarPersiana, activarPuertaExt, NUM_FANS
#include "wifi_local.h" // Para la declaración de secured_client
#include "secret.h"     // Para token, PIN, whitelist (declaración)
#include "UserManager.h" // Gestor de usuarios dinámico
#include "LogManager.h"  // Gestor de logs
#include <Arduino.h>
#include <vector>

// --- Instancia del Bot (UniversalTelegramBot) ---
// secured_client se define en wifi.cpp
// El token se actualizará en setup() con el valor de WiFiManager
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, secured_client);

// --- Variables de Control del Bot ---
unsigned long bot_lasttime = 0;

// --- Estado de Interacción del Usuario ---
enum UserBotStep { STEP_IDLE, STEP_WAITING_PIN, STEP_SELECTING_FANS, STEP_SELECTING_FAN_CMD };

struct UserBotState {
    String chat_id = "";
    UserBotStep current_step = STEP_IDLE;
    String action_pending = "";
    unsigned long timestamp = 0;
    bool fans_selected[NUM_FANS] = {false}; // Selección de ventiladores
};

UserBotState currentUserState; // Estado para UN usuario a la vez
const unsigned long INTERACTION_TIMEOUT = 120000; // Timeout de 2 minutos para interacciones
const unsigned long PIN_WAIT_TIMEOUT = 60000; // Timeout de 1 minuto para PIN

// --- Gestión de Borrado de Mensajes ---
struct MessageToDelete {
    String chat_id;
    int message_id;
    unsigned long delete_at;
};
std::vector<MessageToDelete> messagesToDelete;

void scheduleMessageDeletion(String chat_id, int message_id, unsigned long delay_ms) {
    MessageToDelete msg;
    msg.chat_id = chat_id;
    msg.message_id = message_id;
    msg.delete_at = millis() + delay_ms;
    messagesToDelete.push_back(msg);
}

void processMessageDeletions() {
    if (messagesToDelete.empty()) return;

    unsigned long now = millis();
    for (auto it = messagesToDelete.begin(); it != messagesToDelete.end(); ) {
        if (now >= it->delete_at) {
            String cmd = "deleteMessage?chat_id=" + it->chat_id + "&message_id=" + String(it->message_id);
            // Usamos sendGetToTelegram directamente ya que la librería no expone deleteMessage
            bot.sendGetToTelegram(bot.buildCommand(cmd));
            it = messagesToDelete.erase(it);
        } else {
            ++it;
        }
    }
}

// --- Textos Comandos Ventilador ---
const char* fanCmdTexts[] = {
    "1.ON/OFF MOTOR", "2.ON/OFF LUZ", "3.Velocidad 1", "4.Velocidad 2",
    "5.Velocidad 3", "6.Velocidad 4", "7.Velocidad 5", "8.Velocidad 6"
};
const int numFanCommands = sizeof(fanCmdTexts) / sizeof(fanCmdTexts[0]);
// extern void activarFan(int fanIndex, int commandIndex);


// --- Funciones Auxiliares del Bot ---

// Verifica si un usuario está en la whitelist (Usando UserManager)
bool isUserAllowed(String chat_id) {
  if (userManager.isAuthorized(chat_id)) {
      return true;
  }
  Serial.println("Usuario " + chat_id + " NO permitido.");
  return false;
}

// Limpia/Resetea el estado de interacción del usuario actual
void resetUserState() {
    //Serial.println("Reseteando estado para: " + currentUserState.chat_id); // Log opcional
    currentUserState.chat_id = "";
    currentUserState.current_step = STEP_IDLE;
    currentUserState.action_pending = "";
    currentUserState.timestamp = 0;
    for (int i = 0; i < NUM_FANS; ++i) {
        currentUserState.fans_selected[i] = false; // Resetear selección
    }
}

// Envía el menú principal con los 3 botones
void sendMainMenuKeyboard(String chat_id) {
    String welcome = "\U0001F4F6 Control RF - La Regenta\n";
    welcome += "\U000026A0 Bot solo para usuarios autorizados.\n";
    welcome += "Elija una opción:";

    // Construir el teclado JSON
    String keyboardJson = "[";
    // Fila 1
    keyboardJson += "[{ \"text\" : \"\U0001F511\U0001FA9F Abrir Persiana\", \"callback_data\" : \"req_pin_persiana\" }]";
    keyboardJson += "]";

    // Cancelar operación anterior si la hubiera y resetear estado
    if (currentUserState.chat_id == chat_id && currentUserState.current_step != STEP_IDLE) {
        bot.sendMessage(chat_id, "Cancelando operación anterior.", "");
    }
    resetUserState(); // Resetear siempre al mostrar menú principal

    // Enviar mensaje con teclado
    if (!bot.sendMessageWithInlineKeyboard(chat_id, welcome, "", keyboardJson)) {
        Serial.println("Error enviando menú principal con teclado.");
    }
}

// Solicita PIN para persiana/puerta
void requestPin(String chat_id, String action) {
    // Evitar si el bot está ocupado con otro usuario
     if (currentUserState.chat_id != "" && currentUserState.chat_id != chat_id) {
        bot.sendMessage(chat_id, "El sistema está ocupado. Inténtalo más tarde.", ""); return;
    }
    // Evitar si ya se está esperando PIN
    if (currentUserState.current_step == STEP_WAITING_PIN) {
         bot.sendMessage(chat_id, "Ya estoy esperando un PIN.", ""); return;
    }

    resetUserState(); // Limpiar estado anterior
    Serial.println("Solicitando PIN para accion: " + action + " a chat_id: " + chat_id);
    // Establecer nuevo estado de espera
    currentUserState.chat_id = chat_id;
    currentUserState.current_step = STEP_WAITING_PIN;
    currentUserState.action_pending = action;
    currentUserState.timestamp = millis();

    // Construir y enviar mensaje de solicitud
    String message = "\U0001F512 Por favor, introduce el PIN para ";
    if (action == "persiana") message += "abrir la persiana:";
    else if (action == "puerta") message += "abrir la puerta:";
    else message += "continuar:"; // Fallback
    bot.sendMessage(chat_id, message, "");
}


// --- Funciones para Teclados de Ventiladores ---

// Construye y ENVÍA el teclado de selección de ventiladores (SIEMPRE mensaje nuevo)
void sendFanSelectionKeyboard(String chat_id) {
    // Construir texto del mensaje
    String text = "\U0001F32C️ **Selección de Ventiladores**\n";
    text += "Pulsa para seleccionar/deseleccionar.\n";
    text += "Fans actuales: ";
    bool first = true;
    for (int i = 0; i < NUM_FANS; i++) {
        if (currentUserState.fans_selected[i]) {
            if (!first) text += ", "; text += String(i + 1); first = false;
        }
    }
    if (first) text += "Ninguno"; // Si ningún fan está seleccionado

    // Construir teclado JSON
    String keyboardJson = "[";
    // Fila 1: Ventiladores 1 y 2
    keyboardJson += "[{ \"text\" : \"" + String(currentUserState.fans_selected[0] ? "\U00002705" : "\U0000274C") + " Vent.1\", \"callback_data\" : \"fan_sel_0\" },";
    keyboardJson += "{ \"text\" : \"" + String(currentUserState.fans_selected[1] ? "\U00002705" : "\U0000274C") + " Vent.2\", \"callback_data\" : \"fan_sel_1\" }],";
    // Fila 2: Ventiladores 3 y 4
    keyboardJson += "[{ \"text\" : \"" + String(currentUserState.fans_selected[2] ? "\U00002705" : "\U0000274C") + " Vent.3\", \"callback_data\" : \"fan_sel_2\" },";
    keyboardJson += "{ \"text\" : \"" + String(currentUserState.fans_selected[3] ? "\U00002705" : "\U0000274C") + " Vent.4\", \"callback_data\" : \"fan_sel_3\" }],";
    // Fila 3: Siguiente
    keyboardJson += "[{ \"text\" : \"Siguiente \U000027A1️\", \"callback_data\" : \"fan_sel_next\" }]";
    keyboardJson += "]";

    // Actualizar timestamp de interacción
    currentUserState.timestamp = millis();

    // Enviar SIEMPRE como nuevo mensaje (sin edición con UniversalTelegramBot)
    if (!bot.sendMessageWithInlineKeyboard(chat_id, text, "Markdown", keyboardJson)) {
        Serial.println("Error enviando teclado selección fans.");
        resetUserState(); // Resetear estado si falla el envío
    }
}

// Construye y ENVÍA el teclado de comandos del ventilador (SIEMPRE mensaje nuevo)
void sendFanCommandKeyboard(String chat_id) {
    // Construir texto del mensaje
    String text = "\U0001F32C️ **Comandos Ventilador**\n";
    text += "Selecciona comando para fans: ";
     bool first = true;
    for (int i = 0; i < NUM_FANS; i++) {
        if (currentUserState.fans_selected[i]) {
            if (!first) text += ", "; text += String(i + 1); first = false;
        }
    }
    if (first) { // Si no hay ninguno seleccionado
        text += "Ninguno";
        bot.sendMessage(chat_id, "Error: No hay ventiladores seleccionados.", "");
        resetUserState(); // Resetear y salir
        return;
    }

    // Construir teclado JSON
    String keyboardJson = "[";
    for (int i = 0; i < numFanCommands; i += 2) { // Dos botones por fila
        keyboardJson += "[{ \"text\" : \"" + String(fanCmdTexts[i]) + "\", \"callback_data\" : \"fan_cmd_" + String(i) + "\" }";
        if (i + 1 < numFanCommands) { // Si hay un segundo botón para esta fila
            keyboardJson += ",{ \"text\" : \"" + String(fanCmdTexts[i+1]) + "\", \"callback_data\" : \"fan_cmd_" + String(i+1) + "\" }";
        }
        keyboardJson += "]"; // Cerrar fila
        if (i + 2 < numFanCommands) { keyboardJson += ","; } // Coma si hay más filas
    }
    // Añadir botón "Volver" en su propia fila
    keyboardJson += ",[{ \"text\" : \"\U00002B05️ Volver\", \"callback_data\" : \"fan_cmd_back\" }]";
    keyboardJson += "]";

    // Actualizar timestamp
    currentUserState.timestamp = millis();

    // Enviar SIEMPRE como nuevo mensaje
    if (!bot.sendMessageWithInlineKeyboard(chat_id, text, "Markdown", keyboardJson)) {
        Serial.println("Error enviando teclado comandos fans.");
        resetUserState(); // Resetear si falla
    }
}


// --- Procesa comandos de texto (recibidos por el usuario) ---
void processTextMessage(String chat_id, String text, int message_id, String from_name) {
    Serial.println("Procesando texto (msg_id: " + String(message_id) + ") de " + from_name + " (" + chat_id + "): " + text);

    // 1. Comprobar Whitelist
    if (!isUserAllowed(chat_id)) {
        bot.sendMessage(chat_id, "\U0001F6AB Acceso denegado.", "");
        // No se puede borrar el mensaje fácilmente
        return;
    }

    // 2. Comprobar si se estaba esperando un PIN
    if (currentUserState.chat_id == chat_id && currentUserState.current_step == STEP_WAITING_PIN) {
        Serial.println("Mensaje recibido mientras se esperaba PIN para: " + currentUserState.action_pending);
        
        // Programar borrado del mensaje con el PIN (después de 2 segundos)
        scheduleMessageDeletion(chat_id, message_id, 2000);
        Serial.println("INFO: Mensaje de PIN programado para borrado.");

        // Comprobar PIN
        if (text == SECURITY_PIN) {
            Serial.println("PIN Correcto!");
            String success_message = "\U00002705 PIN correcto. ";
            bool action_taken = false; // Para saber si reenviar menú

            // Ejecutar acción pendiente
            if (currentUserState.action_pending == "persiana") {
                success_message += "Activando persiana...";
                bot.sendMessage(chat_id, success_message, "");
                activarPersiana();
                logManager.logAction(chat_id, from_name, "Abrir Persiana");
                action_taken = true;
            } else if (currentUserState.action_pending == "puerta") {
                success_message += "Activando puerta...";
                bot.sendMessage(chat_id, success_message, "");
                activarPuerta();
                logManager.logAction(chat_id, from_name, "Abrir Puerta");
                action_taken = true;
            }

            resetUserState(); // Limpiar estado después de procesar PIN

            // Reenviar menú principal si se hizo una acción
            if (action_taken) {
                delay(500);
                sendMainMenuKeyboard(chat_id);
            }
        } else {
            // PIN Incorrecto
            Serial.println("PIN Incorrecto.");
            bot.sendMessage(chat_id, "\U0000274C PIN incorrecto. Operación cancelada.", "");
            resetUserState(); // Limpiar estado
        }
        return; // El mensaje era un PIN, no procesar más
    }

    // 3. Comprobar si se envió texto durante una interacción de ventiladores
     if (currentUserState.chat_id == chat_id && currentUserState.current_step != STEP_IDLE) {
         bot.sendMessage(chat_id, "Recibí texto inesperado. Cancelando operación actual.", "");
         sendMainMenuKeyboard(chat_id); // Resetea y muestra menú
         return;
     }

    // 4. Procesar comandos normales /start o /menu
    if (text == "/start" || text == "/menu") {
        sendMainMenuKeyboard(chat_id);
    } else if (text == "/abrirPuerta") {
        requestPin(chat_id, "puerta");
    } 
    // --- Comandos de Administración ---
    else if (text.startsWith("/adduser ") && userManager.isAdmin(chat_id)) {
        // Formato: /adduser <id> <nombre>
        int firstSpace = text.indexOf(' ');
        int secondSpace = text.indexOf(' ', firstSpace + 1);
        
        if (firstSpace > 0 && secondSpace > 0) {
            String newId = text.substring(firstSpace + 1, secondSpace);
            String newName = text.substring(secondSpace + 1);
            userManager.addUser(newId, newName);
            bot.sendMessage(chat_id, "Usuario añadido: " + newName + " (" + newId + ")", "");
            logManager.logAction(chat_id, from_name, "Add User: " + newName);
        } else {
            bot.sendMessage(chat_id, "Uso: /adduser <id> <nombre>", "");
        }
    } else if (text.startsWith("/deluser ") && userManager.isAdmin(chat_id)) {
        // Formato: /deluser <id>
        String targetId = text.substring(9);
        targetId.trim();
        if (targetId.length() > 0) {
            userManager.removeUser(targetId);
            bot.sendMessage(chat_id, "Usuario eliminado: " + targetId, "");
            logManager.logAction(chat_id, from_name, "Del User: " + targetId);
        } else {
            bot.sendMessage(chat_id, "Uso: /deluser <id>", "");
        }
    } else if (text == "/listusers" && userManager.isAdmin(chat_id)) {
        bot.sendMessage(chat_id, userManager.listUsers(), "");
    } else if (text == "/logs" && userManager.isAdmin(chat_id)) {
        String logs = logManager.getLogs();
        if (logs.length() == 0) logs = "No hay logs registrados.";
        // Enviar en trozos si es muy largo (aunque 30 logs deberían caber en 4096 chars)
        if (logs.length() > 4000) {
            bot.sendMessage(chat_id, logs.substring(0, 4000), "");
            bot.sendMessage(chat_id, logs.substring(4000), "");
        } else {
            bot.sendMessage(chat_id, logs, "");
        }
    } else if (text == "/adminCommands" && userManager.isAdmin(chat_id)) {
        String help = "🛠 **Comandos de Administrador**:\n\n";
        help += "/adduser <id> <nombre> - Añadir usuario\n";
        help += "/deluser <id> - Eliminar usuario\n";
        help += "/listusers - Listar usuarios\n";
        help += "/logs - Ver registro de actividad\n";
        help += "/adminCommands - Ver esta lista";
        bot.sendMessage(chat_id, help, "");
    } else if (text == "/myid") {
        bot.sendMessage(chat_id, "Tu ID es: " + chat_id, "");
    }
    else {
        // Comando desconocido o texto aleatorio
        bot.sendMessage(chat_id, "Comando no reconocido. Usa /start o /menu.", "");
        // No se puede borrar fácilmente
    }
}

// --- Procesa Callbacks (clicks en botones inline) ---
void processCallbackQuery(String query_id, String chat_id, String data, String from_name) {
  Serial.println("Procesando callback (" + query_id + ") de " + from_name + " (" + chat_id + "): " + data);

  // 1. Comprobar Whitelist
  if (!isUserAllowed(chat_id)) {
      bot.answerCallbackQuery(query_id, "Acceso denegado", true); // Mostrar alerta
      return;
  }

  // --- 2. Manejo PIN (Persiana/Puerta) ---
  if (data == "req_pin_persiana") {
      bot.answerCallbackQuery(query_id); // Quitar loading del botón
      requestPin(chat_id, "persiana");
      return;
  }
  if (data == "req_pin_puerta") {
      bot.answerCallbackQuery(query_id); // Quitar loading
      requestPin(chat_id, "puerta");
      return;
  }

  // --- 3. Manejo Flujo Ventiladores ---

  // Comprobar si el bot está ocupado con otro usuario
  if (currentUserState.chat_id != "" && currentUserState.chat_id != chat_id) {
      bot.answerCallbackQuery(query_id, "Bot ocupado con otro usuario. Inténtalo más tarde.", true);
      return;
  }
  // Iniciar sesión si no había una y llega callback de fan
  if (currentUserState.chat_id == "" && (data.startsWith("menu_fans") || data.startsWith("fan_"))) {
       currentUserState.chat_id = chat_id; // Asignar usuario actual
       Serial.println("Iniciando sesión para ventiladores para chat_id: " + chat_id);
  }
  // Ignorar si es callback de fan para usuario inactivo (raro, pero por si acaso)
  else if (currentUserState.chat_id != chat_id && data.startsWith("fan_")) {
      bot.answerCallbackQuery(query_id, "Sesión expirada o inactiva. Usa /menu.", true);
      return;
  }

  // **Iniciar Selección de Ventiladores** - MODIFICADO
  if (data == "menu_fans") {
      bot.answerCallbackQuery(query_id); // Quitar loading
      
      // Inicializar estado de sesión antes de establecer cualquier otra cosa
      resetUserState(); // Resetea completamente el estado
      currentUserState.chat_id = chat_id;
      currentUserState.current_step = STEP_SELECTING_FANS;
      currentUserState.timestamp = millis();
      
      // Iniciar TODOS seleccionados - Ahora funcionará correctamente
      for (int i = 0; i < NUM_FANS; ++i) {
          currentUserState.fans_selected[i] = true;
      }
      
      sendFanSelectionKeyboard(chat_id); // Enviar NUEVO teclado
      return;
  }

  // **Seleccionar/Deseleccionar un Ventilador**
  if (data.startsWith("fan_sel_") && data != "fan_sel_next" && currentUserState.current_step == STEP_SELECTING_FANS) {
      int fanIndex = data.substring(8).toInt(); // Extraer índice del fan (0-3)
      if (fanIndex >= 0 && fanIndex < NUM_FANS) {
          currentUserState.fans_selected[fanIndex] = !currentUserState.fans_selected[fanIndex]; // Toggle estado
          bot.answerCallbackQuery(query_id); // Quitar loading (SIN texto emergente)
          sendFanSelectionKeyboard(chat_id); // Reenviar teclado actualizado (como nuevo mensaje)
      } else {
          // Índice inválido en callback_data
          bot.answerCallbackQuery(query_id, "Error: Ventilador inválido", true);
      }
      return;
  }

  // **Ir a Selección de Comandos ("Siguiente")** 
  if (data == "fan_sel_next" && currentUserState.current_step == STEP_SELECTING_FANS) {
      // Verificar que al menos un fan esté seleccionado
      bool anySelected = false;
      for(int i=0; i<NUM_FANS; ++i) {
          if(currentUserState.fans_selected[i]) {
              anySelected=true;
              break;
          }
      }
      
      if (anySelected) {
          bot.answerCallbackQuery(query_id); // Quitar loading
          
          // Simplemente cambiar el paso actual
          currentUserState.current_step = STEP_SELECTING_FAN_CMD;
          currentUserState.timestamp = millis();
          
          // Enviar el teclado de comandos como mensaje nuevo
          sendFanCommandKeyboard(chat_id);
      } else {
          // No hay fans seleccionados, mostrar alerta
          bot.answerCallbackQuery(query_id, "Selecciona al menos un ventilador antes de continuar.", true);
      }
      return;
  }

  // **Seleccionar un Comando de Ventilador o Volver**
  if (data.startsWith("fan_cmd_") && currentUserState.current_step == STEP_SELECTING_FAN_CMD) {
      if (data == "fan_cmd_back") { // Botón Volver
          bot.answerCallbackQuery(query_id);
          currentUserState.current_step = STEP_SELECTING_FANS; // Volver al paso anterior
          sendFanSelectionKeyboard(chat_id); // Reenviar teclado de selección
      } else { // Es un comando real (fan_cmd_0, fan_cmd_1, ...)
          int cmdIndex = data.substring(8).toInt(); // Extraer índice del comando (0-7)
          if (cmdIndex >= 0 && cmdIndex < numFanCommands) {
              // Mostrar notificación rápida al usuario
              bot.answerCallbackQuery(query_id, "Enviando comando...");

              // Preparar datos para mensaje de confirmación y llamadas RF
              String commandText = fanCmdTexts[cmdIndex];
              String selectedFansStr = "";
              bool firstFan = true;
              int sentCount = 0;

              // Iterar y enviar comando a los ventiladores seleccionados
              Serial.println("--- Ejecutando Comando Fan por Telegram ---");
              Serial.println("Comando: " + commandText + " (Idx: " + String(cmdIndex) + ")");
              for (int fanIdx = 0; fanIdx < NUM_FANS; ++fanIdx) {
                  if (currentUserState.fans_selected[fanIdx]) {
                      // Construir string para mensaje de confirmación
                      if (!firstFan) selectedFansStr += ",";
                      selectedFansStr += String(fanIdx + 1);
                      firstFan = false;

                      // LLAMADA A LA ACCIÓN RF
                      Serial.print("  -> Enviando a Fan "); Serial.println(fanIdx + 1);
                      activarFan(fanIdx, cmdIndex);
                      sentCount++;
                      delay(50); // Pausa entre envíos RF
                  }
              }
               Serial.println("--- Fin Ejecución Comando Fan ---");

              // LOG
              logManager.logAction(chat_id, from_name, "Fan: " + commandText + " -> " + selectedFansStr);

              // Construir y enviar mensaje de confirmación final
              String confirmMsg = "\U00002714️ Comando '" + commandText + "' enviado a " + String(sentCount) + " vent: " + selectedFansStr;

              resetUserState(); // Resetear estado ANTES de enviar el menú final
              bot.sendMessage(chat_id, confirmMsg, ""); // Enviar confirmación

              delay(500); // Pausa antes de mostrar menú
              sendMainMenuKeyboard(chat_id); // Volver al menú principal

          } else {
              // Índice de comando inválido en callback_data
              bot.answerCallbackQuery(query_id, "Error: Comando inválido", true);
          }
      }
      return; // Callback procesado
  }

  // Si el callback no coincide con ninguna lógica anterior
  Serial.println("Callback no manejado o estado incorrecto: " + data);
  bot.answerCallbackQuery(query_id, "Acción no válida o expirada.", true);
  // Considerar resetear estado si llega un callback inesperado
  // resetUserState();
  // sendMainMenuKeyboard(chat_id);
} // Fin processCallbackQuery


// --- Función Principal de Manejo del Bot ---
void handleBot() {
  // 0. Procesar borrado de mensajes pendientes
  processMessageDeletions();

  // 1. Comprobar timeout de interacción general o PIN
  if (currentUserState.chat_id != "") {
      unsigned long timeoutLimit = (currentUserState.current_step == STEP_WAITING_PIN) ? PIN_WAIT_TIMEOUT : INTERACTION_TIMEOUT;
      if (millis() - currentUserState.timestamp > timeoutLimit) {
          Serial.println("Timeout (" + String(timeoutLimit/1000) + "s) para " + currentUserState.chat_id);
          String timeoutMsg = (currentUserState.current_step == STEP_WAITING_PIN) ?
              "\U000023F0 Timeout PIN. Operación cancelada." :
              "\U000023F0 Timeout interacción. Volviendo al menú.";
          bot.sendMessage(currentUserState.chat_id, timeoutMsg, "");
          if (currentUserState.current_step != STEP_WAITING_PIN) {
               sendMainMenuKeyboard(currentUserState.chat_id); // Resetea y manda menú
          } else {
               resetUserState(); // Solo resetea si era PIN
          }
      }
  }

  // 2. Comprobar nuevos mensajes (Long Polling)
  // getUpdates bloqueará hasta 5s si no hay mensajes, ahorrando CPU y datos
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
  // Procesar mensajes recibidos (si los hay)
  for (int i = 0; i < numNewMessages; i++) {
      // Extraer datos comunes del mensaje
      String chat_id = bot.messages[i].chat_id;
      String from_name = bot.messages[i].from_name; if (from_name == "") from_name = "Guest";
      int message_id = bot.messages[i].message_id;

      // Procesar según el tipo
      if (bot.messages[i].type == F("message")) { // Mensaje de Texto
          String text = bot.messages[i].text;
          processTextMessage(chat_id, text, message_id, from_name);
      } else if (bot.messages[i].type == F("callback_query")) { // Click en Botón Inline
          String query_id = bot.messages[i].query_id;
          
          // CORRECCIÓN: En esta librería, el callback_data se guarda en el campo 'text'
          // No cambiamos esto, pero sí corregimos la lógica de procesamiento
          String data = bot.messages[i].text;
          
          processCallbackQuery(query_id, chat_id, data, from_name);
      } else {
            Serial.println("Tipo msg no manejado: " + bot.messages[i].type);
      }
  } // Fin for mensajes
} // Fin handleBot