#ifndef __USER_MANAGER_H__
#define __USER_MANAGER_H__

#include <Arduino.h>
#include <vector>

struct BotUser {
    String chat_id;
    String name;
};

class UserManager {
public:
    void begin();
    bool isAuthorized(String chat_id);
    void addUser(String chat_id, String name);
    void removeUser(String chat_id);
    String listUsers();
    bool isAdmin(String chat_id);
    String getAdminId();

private:
    void loadUsers();
    void saveUsers();
    std::vector<BotUser> _users;
    const char* _admin_id = "399392546"; // Lucas
};

extern UserManager userManager;

#endif
