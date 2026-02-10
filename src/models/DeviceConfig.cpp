#include "models/DeviceConfig.h"
#include <string.h>

DeviceConfig::DeviceConfig() 
    : mqttPort(1883)
    , sensitivityThreshold(0.5f)
    , alertDuration(5000)
    , debounceWindow(1000) {
    memset(wifiSSID, 0, sizeof(wifiSSID));
    memset(wifiPassword, 0, sizeof(wifiPassword));
    memset(mqttBrokerIP, 0, sizeof(mqttBrokerIP));
    memset(deviceID, 0, sizeof(deviceID));
    memset(mqttTopic, 0, sizeof(mqttTopic));
}

bool DeviceConfig::isValid() const {
    // Check required fields are populated
    if (strlen(wifiSSID) == 0) {
        return false;
    }
    
    if (strlen(mqttBrokerIP) == 0) {
        return false;
    }
    
    if (strlen(deviceID) == 0) {
        return false;
    }
    
    if (strlen(mqttTopic) == 0) {
        return false;
    }
    
    if (mqttPort == 0 || mqttPort > 65535) {
        return false;
    }
    
    if (sensitivityThreshold <= 0.0f) {
        return false;
    }
    
    return true;
}
