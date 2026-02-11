#include "services/MQTTService.h"
#include "utils/Logger.h"
#include <WiFi.h>

static const uint32_t MQTT_TIMEOUT = 5000;  // 5 second timeout

// Static instance for callback routing
MQTTService* MQTTService::sInstance = nullptr;


MQTTService::MQTTService(const DeviceConfig& config)
    : mConfig(config)
    , mWiFiClient()
    , mMQTTClient(mWiFiClient)
    , mCallback(nullptr)
    , mLastConnectAttempt(0) {
    
    // Initialize the event topic map
    dictEventTopic[EventTypeID::MOVEMENT_EVENT] = mConfig.mqttTopic;
    dictEventTopic[EventTypeID::HARDWARE_EVENT] = mConfig.mqttHardwareTopic;
    
    // Parse IP address from string
    IPAddress brokerIP;
    if (brokerIP.fromString(mConfig.mqttBrokerIP)) {
        mMQTTClient.setServer(brokerIP, mConfig.mqttPort);
        Logger::debug("MQTT server configured:");
        Logger::debug(mConfig.mqttBrokerIP);
    } else {
        Logger::error("Failed to parse MQTT broker IP");
    }
    
    mMQTTClient.setCallback(messageCallback);
    sInstance = this;
}

bool MQTTService::connect() {
    mLastConnectAttempt = millis();
    
    bool connected = mMQTTClient.connect(mConfig.deviceID);
    return connected;
}

bool MQTTService::publish(const IEvent& event) {
    if (!mMQTTClient.connected()) {
        return false;
    }
    
    char jsonBuffer[256];
    if (!event.toJSON(jsonBuffer, sizeof(jsonBuffer))) {
        return false;
    }
    
    EventTypeID typeID = event.getEventTypeID();
    const char* topic = dictEventTopic.count(typeID) ? dictEventTopic[typeID].c_str() : "";
    
    return mMQTTClient.publish(topic, jsonBuffer);
}

bool MQTTService::subscribe(void (*callback)(const IEvent&)) {
    if (!mMQTTClient.connected()) {
        return false;
    }
    
    mCallback = callback;
    return mMQTTClient.subscribe(mConfig.mqttTopic);
}

void MQTTService::loop() {
    mMQTTClient.loop();
}

bool MQTTService::maintainConnection(ConnectionStatus& status) {
    if (mMQTTClient.connected()) {
        status.mqttConnected = true;
        return true;
    }
    
    status.mqttConnected = false;
    
    // Throttle reconnection attempts
    if (millis() - mLastConnectAttempt < MQTT_TIMEOUT) {
        return false;
    }
    
    Logger::info("Attempting MQTT connection...");
    if (attemptConnection()) {
        Logger::info("MQTT connected");
        status.mqttConnected = true;
        //status.las = millis();
        return true;
    }
    
    Logger::error("MQTT connection failed");
    //status.lastMqttConnectAttempt = millis();
    return false;
}

bool MQTTService::isConnected() {
    return mMQTTClient.connected();
}

bool MQTTService::attemptConnection() {
    mLastConnectAttempt = millis();
    return mMQTTClient.connect(mConfig.deviceID);
}

void MQTTService::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (!sInstance || !sInstance->mCallback) {
        return;
    }
    
    // Create null-terminated string from payload
    char message[256];
    size_t len = (length < sizeof(message) - 1) ? length : sizeof(message) - 1;
    memcpy(message, payload, len);
    message[len] = '\0';
    
    Logger::debug("Received MQTT message");
    Logger::debug(topic);
    Logger::debug(message);
    
    // Parse the message based on topic
    // For now, assuming movement events on the main topic
    MovementEvent event;
    if (event.fromJSON(message)) {
        sInstance->mCallback(event);
    }
}