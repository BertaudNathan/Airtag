#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

/**
 * @brief Simple logging utility for serial output
 * 
 * Provides consistent logging with severity levels.
 */
class Logger {
public:
    enum Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };
    
    /**
     * @brief Initialize logger with serial baud rate
     * @param baudRate Serial communication speed (default 115200)
     */
    static void begin(unsigned long baudRate = 115200);
    
    /**
     * @brief Log debug message
     */
    static void debug(const char* message);
    
    /**
     * @brief Log informational message
     */
    static void info(const char* message);
    
    /**
     * @brief Log warning message
     */
    static void warning(const char* message);
    
    /**
     * @brief Log error message
     */
    static void error(const char* message);
    
    /**
     * @brief Log with specific level
     */
    static void log(Level level, const char* message);
    
private:
    static const char* getLevelString(Level level);
};

#endif // LOGGER_H
