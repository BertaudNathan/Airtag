#include "services/HardwareService.h"
#include <string.h>
#include <stdio.h>
#include <Arduino.h>
#include <esp_system.h>

HardwareService::HardwareService() {
}

HardwareService::~HardwareService() {
}

char* HardwareService::getRAMUsage() {
    static char buffer[16];
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    uint8_t ramUsagePercent = (usedHeap * 100) / totalHeap;
    snprintf(buffer, sizeof(buffer), "%d", ramUsagePercent);
    return buffer;
}

char* HardwareService::getCPUUsage() {
    static char buffer[16];
    // ESP32 doesn't directly provide CPU usage, return frequency instead
    uint32_t cpuFreqMHz = ESP.getCpuFreqMHz();
    snprintf(buffer, sizeof(buffer), "%lu", cpuFreqMHz);
    return buffer;
}

char* HardwareService::getUptime() {
    static char buffer[32];
    unsigned long uptimeSeconds = millis() / 1000;
    snprintf(buffer, sizeof(buffer), "%lu", uptimeSeconds);
    return buffer;
}

char* HardwareService::getFreeHeap() {
    static char buffer[16];
    uint32_t freeHeap = ESP.getFreeHeap();
    snprintf(buffer, sizeof(buffer), "%lu", freeHeap);
    return buffer;
}

char* HardwareService::getDataAsJson() {
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), 
             "{\"ramUsage\":\"%s%%\",\"cpuFreqMHz\":\"%s\",\"uptime\":\"%s\",\"freeHeap\":\"%s\"}", 
             getRAMUsage(), getCPUUsage(), getUptime(), getFreeHeap());
    return buffer;
}