#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include <Arduino.h>

/**
 * @brief Device configuration parameters
 * 
 * Contains all runtime configuration for WiFi, MQTT, and device behavior.
 * Values should be loaded from constants or future EEPROM storage.
 */
struct DeviceConfig {
    char wifiSSID[64];              // WiFi network name
    char wifiPassword[64];          // WiFi password
    char mqttBrokerIP[16];          // MQTT broker IP address
    uint16_t mqttPort;              // MQTT broker port (default 1883)
    char deviceID[32];              // Unique device identifier
    char mqttTopic[64];             // MQTT topic for pub/sub
    char mqttHardwareTopic[64];       // MQTT topic for hardware status (optional)
    float sensitivityThreshold;     // Motion threshold in G-force
    uint32_t alertDuration;         // Alert duration in milliseconds
    uint32_t debounceWindow;        // Debounce time window in ms

    
    /**
     * @brief Default constructor with sensible defaults
     */
    DeviceConfig();
    
    /**
     * @brief Validate configuration parameters
     * @return true if all required fields are populated
     */
    bool isValid() const;
};

#endif // DEVICE_CONFIG_H
