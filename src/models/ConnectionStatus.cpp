#include "models/ConnectionStatus.h"

// Constants for exponential backoff
static const uint32_t MIN_RETRY_DELAY = 1000;    // 1 second
static const uint32_t MAX_RETRY_DELAY = 300000;  // 5 minutes
static const uint8_t MAX_RETRY_ATTEMPTS = 10;

ConnectionStatus::ConnectionStatus() 
    : wifiConnected(false)
    , wifiRSSI(0)
    , mqttConnected(false)
    , lastReconnectAttempt(0)
    , reconnectAttempts(0)
    , nextRetryDelay(MIN_RETRY_DELAY) {
}

void ConnectionStatus::updateWiFiStatus(bool connected, int rssi) {
    wifiConnected = connected;
    wifiRSSI = connected ? rssi : 0;
    
    if (connected) {
        resetRetryCounters();
    }
}

void ConnectionStatus::updateMQTTStatus(bool connected) {
    mqttConnected = connected;
    
    if (connected) {
        resetRetryCounters();
    }
}

void ConnectionStatus::recordFailedAttempt() {
    lastReconnectAttempt = millis();
    
    if (reconnectAttempts < MAX_RETRY_ATTEMPTS) {
        reconnectAttempts++;
    }
    
    nextRetryDelay = calculateBackoffDelay();
}

void ConnectionStatus::resetRetryCounters() {
    reconnectAttempts = 0;
    nextRetryDelay = MIN_RETRY_DELAY;
}

uint32_t ConnectionStatus::calculateBackoffDelay() const {
    // Exponential backoff: delay = MIN * (2 ^ attempts)
    uint32_t delay = MIN_RETRY_DELAY * (1 << reconnectAttempts);
    
    // Cap at maximum delay
    if (delay > MAX_RETRY_DELAY) {
        delay = MAX_RETRY_DELAY;
    }
    
    return delay;
}
