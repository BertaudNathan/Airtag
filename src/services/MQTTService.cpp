#include "services/MQTTService.h"

static const uint32_t MQTT_TIMEOUT = 5000;  // 5 second timeout

// Static instance for callback routing
MQTTService* MQTTService::sInstance = nullptr;

MQTTService::MQTTService(const DeviceConfig& config)
    : mConfig(config)
    , mWiFiClient()
    , mMQTTClient(mWiFiClient)
    , mCallback(nullptr)
    , mLastConnectAttempt(0) {
    
    mMQTTClient.setServer(mConfig.mqttBrokerIP, mConfig.mqttPort);
    mMQTTClient.setCallback(messageCallback);
    sInstance = this;
}

bool MQTTService::connect() {
    mLastConnectAttempt = millis();
    
    bool connected = mMQTTClient.connect(mConfig.deviceID);
    return connected;
}

bool MQTTService::publish(const MovementEvent& event) {
    if (!mMQTTClient.connected()) {
        return false;
    }
    
    char jsonBuffer[256];
    if (!event.toJSON(jsonBuffer, sizeof(jsonBuffer))) {
        return false;
    }
    
    return mMQTTClient.publish(mConfig.mqttTopic, jsonBuffer);
}

bool MQTTService::subscribe(void (*callback)(const MovementEvent&)) {
    if (!mMQTTClient.connected()) {
        return false;
    }
    
    mCallback = callback;
    return mMQTTClient.subscribe(mConfig.mqttTopic);
}

void MQTTService::loop() {
    if (mMQTTClient.connected()) {
        mMQTTClient.loop();
    }
}

bool MQTTService::maintainConnection(ConnectionStatus& status) {
    if (mMQTTClient.connected()) {
        status.updateMQTTStatus(true);
        return true;
    }
    
    // Not connected - check if we should retry
    unsigned long currentTime = millis();
    if (currentTime - mLastConnectAttempt < status.nextRetryDelay) {
        status.updateMQTTStatus(false);
        return false;
    }
    
    // Attempt reconnection
    mLastConnectAttempt = currentTime;
    bool reconnected = attemptConnection();
    
    if (reconnected) {
        status.updateMQTTStatus(true);
    } else {
        status.updateMQTTStatus(false);
        status.recordFailedAttempt();
    }
    
    return reconnected;
}

bool MQTTService::isConnected() const {
    return mMQTTClient.connected();
}

bool MQTTService::attemptConnection() {
    return mMQTTClient.connect(mConfig.deviceID);
}

void MQTTService::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (!sInstance || !sInstance->mCallback) {
        return;
    }
    
    // Null-terminate payload
    char buffer[256];
    size_t copyLen = (length < sizeof(buffer) - 1) ? length : sizeof(buffer) - 1;
    memcpy(buffer, payload, copyLen);
    buffer[copyLen] = '\0';
    
    // Deserialize and invoke callback
    MovementEvent event;
    if (event.fromJSON(buffer)) {
        sInstance->mCallback(event);
    }
}
