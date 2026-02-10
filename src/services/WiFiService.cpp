#include "services/WiFiService.h"
#include "utils/Logger.h"

static const uint32_t WIFI_TIMEOUT = 10000;  // 10 second timeout

WiFiService::WiFiService(const DeviceConfig& config)
    : mConfig(config)
    , mLastConnectAttempt(0) {
}

bool WiFiService::connect() {
    char logMsg[128];
    
    // Disconnect any previous connection
    WiFi.disconnect(true);
    delay(500);
    
    // Set WiFi mode and hostname
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(mConfig.deviceID);
    
    snprintf(logMsg, sizeof(logMsg), "Connecting to: %s", mConfig.wifiSSID);
    Logger::info(logMsg);
    
    // Begin connection
    WiFi.begin(mConfig.wifiSSID, mConfig.wifiPassword);
    mLastConnectAttempt = millis();
    
    // Wait for connection with detailed status logging
    unsigned long startTime = millis();
    int lastStatus = -1;
    
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_TIMEOUT) {
            int finalStatus = WiFi.status();
            const char* statusText = "UNKNOWN";
            switch(finalStatus) {
                case WL_IDLE_STATUS: statusText = "IDLE"; break;
                case WL_NO_SSID_AVAIL: statusText = "NO_SSID_AVAIL"; break;
                case WL_SCAN_COMPLETED: statusText = "SCAN_COMPLETED"; break;
                case WL_CONNECTED: statusText = "CONNECTED"; break;
                case WL_CONNECT_FAILED: statusText = "CONNECT_FAILED"; break;
                case WL_CONNECTION_LOST: statusText = "CONNECTION_LOST"; break;
                case WL_DISCONNECTED: statusText = "DISCONNECTED"; break;
            }
            snprintf(logMsg, sizeof(logMsg), "WiFi timeout! Status: %d (%s)", finalStatus, statusText);
            Logger::error(logMsg);
            return false;
        }
        
        int currentStatus = WiFi.status();
        if (currentStatus != lastStatus) {
            const char* statusText = "UNKNOWN";
            switch(currentStatus) {
                case WL_IDLE_STATUS: statusText = "IDLE"; break;
                case WL_NO_SSID_AVAIL: statusText = "NO_SSID_AVAIL"; break;
                case WL_SCAN_COMPLETED: statusText = "SCAN_COMPLETED"; break;
                case WL_CONNECT_FAILED: statusText = "CONNECT_FAILED"; break;
                case WL_CONNECTION_LOST: statusText = "CONNECTION_LOST"; break;
                case WL_DISCONNECTED: statusText = "DISCONNECTED"; break;
            }
            snprintf(logMsg, sizeof(logMsg), "Status: %d (%s)", currentStatus, statusText);
            Logger::debug(logMsg);
            lastStatus = currentStatus;
        }
        
        delay(500);
    }
    
    Logger::info("WiFi connected!");
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
