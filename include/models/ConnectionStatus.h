#ifndef CONNECTION_STATUS_H
#define CONNECTION_STATUS_H

#include <Arduino.h>

/**
 * @brief Tracks WiFi and MQTT connection status
 * 
 * Contains current connectivity state and retry attempt tracking
 * for robust connection recovery.
 */
struct ConnectionStatus {
    bool wifiConnected;                 // WiFi connection state
    int wifiRSSI;                       // Signal strength in dBm
    bool mqttConnected;                 // MQTT broker connection state
    unsigned long lastReconnectAttempt; // Timestamp of last retry (millis)
    uint8_t reconnectAttempts;          // Number of consecutive failures
    uint32_t nextRetryDelay;            // Next backoff delay in ms
    
    /**
     * @brief Default constructor initializes to disconnected state
     */
    ConnectionStatus();
    
    /**
     * @brief Update WiFi connection status
     * @param connected Current WiFi state
     * @param rssi Signal strength (if connected)
     */
    void updateWiFiStatus(bool connected, int rssi = 0);
    
    /**
     * @brief Update MQTT connection status
     * @param connected Current MQTT state
     */
    void updateMQTTStatus(bool connected);
    
    /**
     * @brief Record failed connection attempt
     */
    void recordFailedAttempt();
    
    /**
     * @brief Reset retry counters after successful connection
     */
    void resetRetryCounters();
    
    /**
     * @brief Calculate next retry delay with exponential backoff
     * @return Delay in milliseconds before next retry
     */
    uint32_t calculateBackoffDelay() const;
};

#endif // CONNECTION_STATUS_H
