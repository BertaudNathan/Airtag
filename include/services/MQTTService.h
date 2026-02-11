#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <PubSubClient.h>
#include <WiFiClient.h>
#include "models/DeviceConfig.h"
#include "models/HardwareEvent.h"
#include "models/MovementEvent.h"
#include "models/ConnectionStatus.h"
#include "interfaces/IEvent.h"
#include <map>
#include <string>
#include <typeindex>

/**
 * @brief Manages MQTT pub/sub operations
 * 
 * Wraps PubSubClient with reconnection logic and message serialization.
 */
class MQTTService {
public:
    /**
     * @brief Construct service with configuration
     * @param config Device configuration with broker settings
     */
    MQTTService(const DeviceConfig& config);
    
    /**
     * @brief Connect to MQTT broker
     * @return true if connection successful
     */
    bool connect();
    
    /**
     * @brief Publish movement event to configured topic
     * @param event Event to publish
     * @return true if publish successful
     */
    bool publish(const IEvent& event);
    
    /**
     * @brief Subscribe to configured topic with callback
     * @param callback Function to call when message received
     * @return true if subscription successful
     */
    bool subscribe(void (*callback)(const IEvent&));
    
    /**
     * @brief Process MQTT events (call in loop)
     */
    void loop();
    
    /**
     * @brief Check connection and reconnect if needed
     * @param status Connection status to update
     * @return true if connected
     */
    bool maintainConnection(ConnectionStatus& status);
    
    /**
     * @brief Check if connected to broker
     * @return true if connected
     */
    bool isConnected();
    
private:
    DeviceConfig mConfig;
    WiFiClient mWiFiClient;
    PubSubClient mMQTTClient;
    void (*mCallback)(const IEvent&);
    unsigned long mLastConnectAttempt;
    std::map<std::type_index, std::string> dictEventTopic = {
        {typeid(MovementEvent), mConfig.mqttTopic},
        {typeid(HardwareEvent), mConfig.mqttHardwareTopic}
    };
    
    bool attemptConnection();
    static void messageCallback(char* topic, byte* payload, unsigned int length);
    static MQTTService* sInstance;
};

#endif // MQTT_SERVICE_H
