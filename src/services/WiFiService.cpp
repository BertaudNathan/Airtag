#include "services/WiFiService.h"

static const uint32_t WIFI_TIMEOUT = 10000;  // 10 second timeout

WiFiService::WiFiService(const DeviceConfig& config)
    : mConfig(config)
    , mLastConnectAttempt(0) {
}

bool WiFiService::connect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(mConfig.wifiSSID, mConfig.wifiPassword);
    
    mLastConnectAttempt = millis();
    
    // Wait for connection with timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_TIMEOUT) {
            return false;
        }
        delay(100);
    }
    
    return true;
}

bool WiFiService::maintainConnection(ConnectionStatus& status) {
    bool connected = (WiFi.status() == WL_CONNECTED);
    
    if (connected) {
        status.updateWiFiStatus(true, WiFi.RSSI());
        return true;
    }
    
    // Not connected - check if we should retry
    unsigned long currentTime = millis();
    if (currentTime - mLastConnectAttempt < status.nextRetryDelay) {
        // Not time to retry yet
        status.updateWiFiStatus(false);
        return false;
    }
    
    // Attempt reconnection
    mLastConnectAttempt = currentTime;
    bool reconnected = attemptConnection();
    
    if (reconnected) {
        status.updateWiFiStatus(true, WiFi.RSSI());
    } else {
        status.updateWiFiStatus(false);
        status.recordFailedAttempt();
    }
    
    return reconnected;
}

int WiFiService::getRSSI() const {
    if (!isConnected()) {
        return 0;
    }
    return WiFi.RSSI();
}

bool WiFiService::isConnected() const {
    return (WiFi.status() == WL_CONNECTED);
}

bool WiFiService::attemptConnection() {
    WiFi.disconnect();
    delay(100);
    WiFi.begin(mConfig.wifiSSID, mConfig.wifiPassword);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_TIMEOUT) {
            return false;
        }
        delay(100);
    }
    
    return true;
}
