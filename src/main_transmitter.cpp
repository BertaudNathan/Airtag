#include <Arduino.h>
#include <Wire.h>
#include "models/DeviceConfig.h"
#include "models/ConnectionStatus.h"
#include "drivers/MPU6050Driver.h"
#include "drivers/LEDDriver.h"
#include "services/AccelerometerService.h"
#include "services/WiFiService.h"
#include "services/MQTTService.h"
#include "utils/Logger.h"
#include "services/HardwareService.h"

#pragma region Configuration
#define STATUS_LED_PIN 2
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 27

DeviceConfig config;
void setupConfiguration()
{
    strcpy(config.wifiSSID, "SamsungA52s");
    strcpy(config.wifiPassword, "nathanleboss");
    strcpy(config.mqttBrokerIP, "192.168.54.82"); // Raspberry Pi IP
    config.mqttPort = 1883;
    strcpy(config.deviceID, "airtag_transmitter_01");
    strcpy(config.mqttTopic, "airtag/motion");
    strcpy(config.mqttHardwareTopic, "airtag/hardware");
    config.sensitivityThreshold = 2.0f; // 2.0G - Threshold for motion detection
    config.debounceWindow = 2000;       // 2 seconds - Debounce window
}

#pragma endregion

// Global objects
MPU6050Driver *accelDriver = nullptr;
LEDDriver *statusLED = nullptr;
AccelerometerService *accelService = nullptr;
WiFiService *wifiService = nullptr;
MQTTService *mqttService = nullptr;
HardwareService *hardwareService = nullptr;
ConnectionStatus connStatus;

void setup()
{
    Logger::begin(115200);
    Logger::info("=== AirTag Transmitter Starting ===");

    // Setup configuration
    setupConfiguration();

    if (!config.isValid())
    {
        Logger::error("Invalid configuration");
        while (true)
        {
            delay(1000);
        }
    }

    // Initialize hardware
    statusLED = new LEDDriver(STATUS_LED_PIN);
    statusLED->setState(false);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    accelDriver = new MPU6050Driver();
    accelService = new AccelerometerService(accelDriver, config);

    if (!accelService->begin())
    {
        Logger::error("Failed to initialize accelerometer");
        while (true)
        {
            statusLED->setState(true);
            delay(100);
            statusLED->setState(false);
            delay(100);
        }
    }

    hardwareService = new HardwareService();
    Logger::info("Hardware service initialized");

    Logger::info("Accelerometer initialized");

    // Connect to WiFi with multiple attempts
    wifiService = new WiFiService(config);
    Logger::info("Connecting to WiFi...");

    int wifiAttempts = 0;
    const int maxWifiAttempts = 20;
    bool wifiConnected = false;

    while (!wifiConnected && wifiAttempts < maxWifiAttempts)
    {
        wifiAttempts++;
        char attemptMsg[64];
        snprintf(attemptMsg, sizeof(attemptMsg), "WiFi attempt %d/%d", wifiAttempts, maxWifiAttempts);
        Logger::info(attemptMsg);

        if (wifiService->connect())
        {
            wifiConnected = true;
            Logger::info("WiFi connected successfully!");

            char ipMsg[64];
            snprintf(ipMsg, sizeof(ipMsg), "IP: %s", WiFi.localIP().toString().c_str());
            Logger::info(ipMsg);

            // Flash LED 3 times to indicate WiFi success
            for (int i = 0; i < 3; i++)
            {
                statusLED->setState(true);
                delay(200);
                statusLED->setState(false);
                delay(200);
            }

            // Wait for network stack to stabilize
            delay(2000);
        }
        else
        {
            Logger::warning("WiFi connection failed, retrying...");
            WiFi.disconnect(true);
            delay(3000);
        }
    }

    if (!wifiConnected)
    {
        Logger::error("WiFi connection failed after all attempts!");
        Logger::error("Cannot proceed without WiFi. Restarting in 10s...");
        delay(10000);
        ESP.restart();
    }

    // Connect to MQTT broker
    mqttService = new MQTTService(config);
    Logger::info("Connecting to MQTT broker...");

    int attempts = 0;
    while (!mqttService->connect() && attempts < 10)
    {
        Logger::warning("MQTT connection failed, retrying...");
        attempts++;
        delay(2000);
    }

    if (mqttService->isConnected())
    {
        Logger::info("MQTT connected");
    }
    else
    {
        Logger::error("MQTT connection failed after 10 attempts");
    }
    statusLED->setState(true);
    delay(500);
    statusLED->setState(false);

    Logger::info("Setup complete - monitoring for motion");
}

void loop()
{
    // Maintain network connections
    wifiService->maintainConnection(connStatus);
    mqttService->maintainConnection(connStatus);

    // Send hardware data every 8 ticks
    static uint8_t tickCounter = 0;
    if (++tickCounter >= 8) {
        tickCounter = 0;
        HardwareEvent hwEvent(config.deviceID, millis(), hardwareService->getRAMUsage(), hardwareService->getCPUUsage(), hardwareService->getUptime(), hardwareService->getFreeHeap());
        mqttService->publish(hwEvent);
    }
    
    mqttService->loop();
    // Update status LED based on connectivity
    if (connStatus.mqttConnected && connStatus.wifiConnected)
    {
        statusLED->setState(false); // Off when fully connected
    }
    else
    {
        // Blink when not fully connected
        static unsigned long lastBlink = 0;
        if (millis() - lastBlink > 500)
        {
            statusLED->toggle();
            lastBlink = millis();
        }
    }

    // Detect motion
    if (accelService->detectMotion())
    {
        MovementEvent event = accelService->getLastEvent();

        Logger::info("Motion detected!");

        if (mqttService->isConnected())
        {
            if (mqttService->publish(event))
            {
                Logger::info("Event published to MQTT");

                // Flash LED to indicate successful publish
                statusLED->setState(true);
                delay(100);
                statusLED->setState(false);
            }
            else
            {
                Logger::error("Failed to publish event");
            }
        }
        else
        {
            Logger::warning("MQTT not connected, cannot publish");
        }
    }

    delay(50); // 20Hz sampling rate
}
