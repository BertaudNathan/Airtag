#include <Arduino.h>
#include "models/DeviceConfig.h"
#include "models/ConnectionStatus.h"
#include "models/MovementEvent.h"
#include "drivers/LEDDriver.h"
#include "drivers/BuzzerDriver.h"
#include "services/AlertService.h"
#include "services/WiFiService.h"
#include "services/MQTTService.h"
#include "utils/Logger.h"
#include "services/HardwareService.h"

// Pin definitions
#define STATUS_LED_PIN 2
#define ALERT_LED_PIN 4
#define BUZZER_PIN 5

// Configuration - Update these values for your network
DeviceConfig config;
void setupConfiguration() {
    strcpy(config.wifiSSID, "SamsungA52s");
    strcpy(config.wifiPassword, "nathanleboss");
    strcpy(config.mqttBrokerIP, "192.168.54.45");  // Raspberry Pi IP
    config.mqttPort = 1883;
    strcpy(config.deviceID, "airtag_receiver_01");
    strcpy(config.mqttTopic, "airtag/motion");
    strcpy(config.mqttHardwareTopic, "airtag/hardware");
    config.alertDuration = 500;  // 0.5 second alert
}

// Global objects
LEDDriver* statusLED = nullptr;
LEDDriver* alertLED = nullptr;
BuzzerDriver* buzzer = nullptr;
AlertService* alertService = nullptr;
WiFiService* wifiService = nullptr;
MQTTService* mqttService = nullptr;
HardwareService *hardwareService = nullptr;
ConnectionStatus connStatus;

// MQTT callback function
void onMotionDetected(const IEvent& baseEvent) {
    Logger::info("Motion event received!");
    const MovementEvent& event = static_cast<const MovementEvent&>(baseEvent);
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Device: %s, Magnitude: %.2f", 
             event.deviceID, event.accelerationMagnitude);
    Logger::info(buffer);
    
    // Trigger alert
    alertService->triggerAlert(config.alertDuration, event);
}

void setup() {
    Logger::begin(115200);
    Logger::info("=== AirTag Receiver Starting ===");
    
    // Setup configuration
    setupConfiguration();
    
    if (!config.isValid()) {
        Logger::error("Invalid configuration");
        while (true) { delay(1000); }
    }
    
    // Initialize hardware
    statusLED = new LEDDriver(STATUS_LED_PIN);
    alertLED = new LEDDriver(ALERT_LED_PIN);
    buzzer = new BuzzerDriver(BUZZER_PIN, 0);
    
    statusLED->setState(false);
    alertLED->setState(false);
    
    alertService = new AlertService(alertLED, buzzer);
    
    hardwareService = new HardwareService();
    Logger::info("Hardware service initialized");
    
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
    
    // Connect to MQTT broker and subscribe
    mqttService = new MQTTService(config);
    Logger::info("Connecting to MQTT broker...");
    
    if (mqttService->connect()) {
        Logger::info("MQTT connected");
        
        if (mqttService->subscribe(onMotionDetected)) {
            Logger::info("Subscribed to motion topic");
            statusLED->setState(true);
            delay(500);
            statusLED->setState(false);
        } else {
            Logger::error("Failed to subscribe");
        }
    } else {
        Logger::warning("MQTT connection failed, will retry");
    }
    
    Logger::info("Setup complete - listening for motion events");
}

void loop() {
    // Maintain network connections
    wifiService->maintainConnection(connStatus);
    static bool wasConnected = false;
    if (mqttService->maintainConnection(connStatus)) {
        // Resubscribe if we just reconnected
        
        if (!wasConnected) {
            mqttService->subscribe( onMotionDetected);
            Logger::info("Resubscribed after reconnection");
        }
        wasConnected = true;
    } else {
        wasConnected = false;
    }
    
    // Send hardware data every 8 ticks
    static uint8_t tickCounter = 0;
    if (++tickCounter >= 8) {
        tickCounter = 0;
        HardwareEvent hwEvent(config.deviceID, millis(), hardwareService->getRAMUsage(), hardwareService->getCPUUsage(), hardwareService->getUptime(), hardwareService->getFreeHeap());
        mqttService->publish(hwEvent);
    }
    
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
    
    // Update alert service
    alertService->update();
    
    delay(10);  // Small delay for responsiveness
}
