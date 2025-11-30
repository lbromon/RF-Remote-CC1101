#ifndef __LOG_MANAGER_H__
#define __LOG_MANAGER_H__

#include <Arduino.h>
#include <vector>

struct LogEntry {
    String timestamp;
    String userId;
    String userName;
    String action;
};

class LogManager {
public:
    void begin();
    void logAction(String userId, String userName, String action);
    String getLogs();
    void clearLogs();

private:
    void loadLogs();
    void saveLogs();
    String getCurrentTime();
    std::vector<LogEntry> _logs;
    const int MAX_LOGS = 50; // Límite solicitado
};

extern LogManager logManager;

#endif
