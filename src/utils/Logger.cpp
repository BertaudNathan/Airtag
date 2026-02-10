#include "utils/Logger.h"

void Logger::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
    while (!Serial && millis() < 2000) {
        delay(10);
    }
}

void Logger::debug(const char* message) {
    log(DEBUG, message);
}

void Logger::info(const char* message) {
    log(INFO, message);
}

void Logger::warning(const char* message) {
    log(WARNING, message);
}

void Logger::error(const char* message) {
    log(ERROR, message);
}

void Logger::log(Level level, const char* message) {
    unsigned long timestamp = millis();
    
    Serial.print("[");
    Serial.print(timestamp);
    Serial.print("] ");
    Serial.print(getLevelString(level));
    Serial.print(": ");
    Serial.println(message);
}

const char* Logger::getLevelString(Level level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARNING: return "WARN";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
