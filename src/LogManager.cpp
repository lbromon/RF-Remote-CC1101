#include "LogManager.h"
#include <Preferences.h>
#include <ArduinoJson.h>
#include <time.h>

LogManager logManager;
Preferences logPrefs;

void LogManager::begin() {
    logPrefs.begin("bot_logs", false); // Namespace "bot_logs"
    loadLogs();
}

void LogManager::loadLogs() {
    _logs.clear();
    String jsonStr = logPrefs.getString("logs_json", "[]");
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (!error) {
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject obj : array) {
            LogEntry entry;
            entry.timestamp = obj["t"].as<String>();
            entry.userId = obj["u"].as<String>();
            entry.userName = obj["n"].as<String>();
            entry.action = obj["a"].as<String>();
            _logs.push_back(entry);
        }
    }
}

void LogManager::saveLogs() {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    for (const auto& entry : _logs) {
        JsonObject obj = array.add<JsonObject>();
        obj["t"] = entry.timestamp;
        obj["u"] = entry.userId;
        obj["n"] = entry.userName;
        obj["a"] = entry.action;
    }
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    logPrefs.putString("logs_json", jsonStr);
}

String LogManager::getCurrentTime() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        return "N/A";
    }
    char timeStringBuff[20];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%d/%m %H:%M", &timeinfo);
    return String(timeStringBuff);
}

void LogManager::logAction(String userId, String userName, String action) {
    LogEntry newEntry;
    newEntry.timestamp = getCurrentTime();
    newEntry.userId = userId;
    newEntry.userName = userName;
    newEntry.action = action;
    
    _logs.push_back(newEntry);
    
    // Mantener límite
    while (_logs.size() > MAX_LOGS) {
        _logs.erase(_logs.begin());
    }
    
    saveLogs();
}

String LogManager::getLogs() {
    if (_logs.empty()) return "No hay logs registrados.";
    
    String list = "📜 **Registro de Actividad**:\n";
    for (const auto& entry : _logs) {
        list += "[" + entry.timestamp + "] " + entry.userName + ": " + entry.action + "\n";
    }
    return list;
}

void LogManager::clearLogs() {
    _logs.clear();
    saveLogs();
}
