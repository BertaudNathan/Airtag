#include <Arduino.h>
#include "models/DeviceConfig.h"
#include "models/ConnectionStatus.h"
#include "drivers/MPU6050Driver.h"
#include "drivers/LEDDriver.h"
#include "services/AccelerometerService.h"
#include "services/WiFiService.h"
#include "services/MQTTService.h"
#include "utils/Logger.h"

// Pin definitions
#define STATUS_LED_PIN 2
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Configuration - Update these values for your network
DeviceConfig config;
void setupConfiguration() {
    strcpy(config.wifiSSID, "YOUR_WIFI_SSID");
    strcpy(config.wifiPassword, "YOUR_WIFI_PASSWORD");
    strcpy(config.mqttBrokerIP, "192.168.1.100");  // Raspberry Pi IP
    config.mqttPort = 1883;
    strcpy(config.deviceID, "airtag_transmitter_01");
    strcpy(config.mqttTopic, "airtag/motion");
    config.sensitivityThreshold = 0.5f;  // 0.5G
    config.debounceWindow = 1000;        // 1 second
}

// Global objects
MPU6050Driver* accelDriver = nullptr;
LEDDriver* statusLED = nullptr;
AccelerometerService* accelService = nullptr;
WiFiService* wifiService = nullptr;
MQTTService* mqttService = nullptr;
ConnectionStatus connStatus;

void setup() {
    Logger::begin(115200);
    Logger::info("=== AirTag Transmitter Starting ===");
    
    // Setup configuration
    setupConfiguration();
    
    if (!config.isValid()) {
        Logger::error("Invalid configuration");
        while (true) { delay(1000); }
    }
    
    // Initialize hardware
    statusLED = new LEDDriver(STATUS_LED_PIN);
    statusLED->setState(false);
    
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    accelDriver = new MPU6050Driver();
    accelService = new AccelerometerService(accelDriver, config);
    
    if (!accelService->begin()) {
        Logger::error("Failed to initialize accelerometer");
        while (true) { 
            statusLED->setState(true);
            delay(100);
            statusLED->setState(false);
            delay(100);
        }
    }
    
    Logger::info("Accelerometer initialized");
    
    // Connect to WiFi
    wifiService = new WiFiService(config);
    Logger::info("Connecting to WiFi...");
    
    if (wifiService->connect()) {
        Logger::info("WiFi connected");
        statusLED->setState(true);
        delay(500);
        statusLED->setState(false);
    } else {
        Logger::error("WiFi connection failed");
    }
    
    // Connect to MQTT broker
    mqttService = new MQTTService(config);
    Logger::info("Connecting to MQTT broker...");
    
    if (mqttService->connect()) {
        Logger::info("MQTT connected");
        statusLED->setState(true);
        delay(500);
        statusLED->setState(false);
    } else {
        Logger::warning("MQTT connection failed, will retry");
    }
    
    Logger::info("Setup complete - monitoring for motion");
}

void loop() {
    // Maintain network connections
    wifiService->maintainConnection(connStatus);
    mqttService->maintainConnection(connStatus);
    mqttService->loop();
    
    // Update status LED based on connectivity
    if (connStatus.mqttConnected && connStatus.wifiConnected) {
        statusLED->setState(false);  // Off when fully connected
    } else {
        // Blink when not fully connected
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500) {
            statusLED->toggle();
            lastBlink = millis();
        }
    }
    
    // Detect motion
    if (accelService->detectMotion()) {
        MovementEvent event = accelService->getLastEvent();
        
        Logger::info("Motion detected!");
        
        if (mqttService->isConnected()) {
            if (mqttService->publish(event)) {
                Logger::info("Event published to MQTT");
                
                // Flash LED to indicate successful publish
                statusLED->setState(true);
                delay(100);
                statusLED->setState(false);
            } else {
                Logger::error("Failed to publish event");
            }
        } else {
            Logger::warning("MQTT not connected, cannot publish");
        }
    }
    
    delay(50);  // 20Hz sampling rate
}
