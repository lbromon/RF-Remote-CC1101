#include "UserManager.h"
#include <Preferences.h>
#include <ArduinoJson.h>

UserManager userManager;
Preferences preferences;

void UserManager::begin() {
    preferences.begin("bot_users", false); // Namespace "bot_users", read/write
    loadUsers();
    
    // Asegurar que el admin siempre está en la lista (opcional, pero recomendado)
    if (!isAuthorized(_admin_id)) {
        addUser(_admin_id, "Admin (Lucas)");
    }
}

void UserManager::loadUsers() {
    _users.clear();
    String jsonStr = preferences.getString("users_json", "[]");
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (!error) {
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject obj : array) {
            BotUser user;
            user.chat_id = obj["id"].as<String>();
            user.name = obj["name"].as<String>();
            _users.push_back(user);
        }
    }
}

void UserManager::saveUsers() {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    for (const auto& user : _users) {
        JsonObject obj = array.add<JsonObject>();
        obj["id"] = user.chat_id;
        obj["name"] = user.name;
    }
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    preferences.putString("users_json", jsonStr);
}

bool UserManager::isAuthorized(String chat_id) {
    for (const auto& user : _users) {
        if (user.chat_id == chat_id) return true;
    }
    return false;
}

void UserManager::addUser(String chat_id, String name) {
    if (isAuthorized(chat_id)) return; // Ya existe
    
    BotUser newUser;
    newUser.chat_id = chat_id;
    newUser.name = name;
    _users.push_back(newUser);
    saveUsers();
}

void UserManager::removeUser(String chat_id) {
    for (auto it = _users.begin(); it != _users.end(); ) {
        if (it->chat_id == chat_id) {
            it = _users.erase(it);
            saveUsers();
            return;
        } else {
            ++it;
        }
    }
}

String UserManager::listUsers() {
    String list = "Usuarios Autorizados:\n";
    for (const auto& user : _users) {
        list += "- " + user.name + " (ID: " + user.chat_id + ")\n";
    }
    return list;
}

bool UserManager::isAdmin(String chat_id) {
    return chat_id == String(_admin_id);
}

String UserManager::getAdminId() {
    return String(_admin_id);
}
