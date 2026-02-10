#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <WiFi.h>
#include "models/DeviceConfig.h"
#include "models/ConnectionStatus.h"

/**
 * @brief Manages WiFi connectivity
 * 
 * Handles connection, reconnection, and status monitoring with
 * exponential backoff retry logic.
 */
class WiFiService {
public:
    /**
     * @brief Construct service with configuration
     * @param config Device configuration with WiFi credentials
     */
    WiFiService(const DeviceConfig& config);
    
    /**
     * @brief Attempt to connect to WiFi network
     * @return true if connection successful
     */
    bool connect();
    
    /**
     * @brief Check connection status and attempt reconnect if needed
     * @param status Connection status to update
     * @return true if currently connected
     */
    bool maintainConnection(ConnectionStatus& status);
    
    /**
     * @brief Get current signal strength
     * @return RSSI in dBm
     */
    int getRSSI() const;
    
    /**
     * @brief Check if connected to WiFi
     * @return true if connected
     */
    bool isConnected() const;
    
private:
    DeviceConfig mConfig;
    unsigned long mLastConnectAttempt;
    
    bool attemptConnection();
};

#endif // WIFI_SERVICE_H
