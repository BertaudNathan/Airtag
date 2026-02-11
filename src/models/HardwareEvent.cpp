#include "models/HardwareEvent.h"
#include <Arduino.h>
#include <string.h>
#include <stdio.h>

HardwareEvent::HardwareEvent() 
    : timestamp(0) {
    memset(deviceID, 0, sizeof(deviceID));
    memset(ramUsage, 0, sizeof(ramUsage));
    memset(cpuFreqMHz, 0, sizeof(cpuFreqMHz));
    memset(uptime, 0, sizeof(uptime));
    memset(freeHeap, 0, sizeof(freeHeap));
}

HardwareEvent::HardwareEvent(const char* id, unsigned long ts, const char* ram, const char* cpu, const char* uptimeStr, const char* heap)
    : timestamp(ts) {
    strncpy(deviceID, id, sizeof(deviceID) - 1);
    deviceID[sizeof(deviceID) - 1] = '\0';
    
    strncpy(ramUsage, ram, sizeof(ramUsage) - 1);
    ramUsage[sizeof(ramUsage) - 1] = '\0';
    
    strncpy(cpuFreqMHz, cpu, sizeof(cpuFreqMHz) - 1);
    cpuFreqMHz[sizeof(cpuFreqMHz) - 1] = '\0';
    
    strncpy(uptime, uptimeStr, sizeof(uptime) - 1);
    uptime[sizeof(uptime) - 1] = '\0';
    
    strncpy(freeHeap, heap, sizeof(freeHeap) - 1);
    freeHeap[sizeof(freeHeap) - 1] = '\0';
}

bool HardwareEvent::toJSON(char* buffer, size_t bufferSize) const {
    int written = snprintf(buffer, bufferSize,
        "{\"deviceID\":\"%s\",\"timestamp\":%lu,\"ramUsage\":\"%s\",\"cpuFreqMHz\":\"%s\",\"uptime\":\"%s\",\"freeHeap\":\"%s\"}",
        deviceID, timestamp, ramUsage, cpuFreqMHz, uptime, freeHeap);
    
    return written > 0 && written < (int)bufferSize;
}

bool HardwareEvent::fromJSON(const char* jsonStr) {
    // Simple JSON parsing - in production, use a proper JSON library
    // Format: {"deviceID":"xxx","timestamp":123,"ramUsage":"xx%","cpuFreqMHz":"xxx","uptime":"xxx","freeHeap":"xxx"}
    
    const char* p = jsonStr;
    
    // Extract deviceID
    p = strstr(p, "\"deviceID\":\"");
    if (!p) return false;
    p += 12; // Length of "deviceID":""
    const char* end = strchr(p, '\"');
    if (!end) return false;
    size_t len = end - p;
    if (len >= sizeof(deviceID)) len = sizeof(deviceID) - 1;
    strncpy(deviceID, p, len);
    deviceID[len] = '\0';
    
    // Extract timestamp
    p = strstr(end, "\"timestamp\":");
    if (!p) return false;
    p += 12; // Length of "timestamp":"
    timestamp = strtoul(p, nullptr, 10);
    
    // Extract ramUsage
    p = strstr(p, "\"ramUsage\":\"");
    if (!p) return false;
    p += 12; // Length of "ramUsage":""
    end = strchr(p, '\"');
    if (!end) return false;
    len = end - p;
    if (len >= sizeof(ramUsage)) len = sizeof(ramUsage) - 1;
    strncpy(ramUsage, p, len);
    ramUsage[len] = '\0';
    
    // Extract cpuFreqMHz
    p = strstr(end, "\"cpuFreqMHz\":\"");
    if (!p) return false;
    p += 14; // Length of "cpuFreqMHz":""
    end = strchr(p, '\"');
    if (!end) return false;
    len = end - p;
    if (len >= sizeof(cpuFreqMHz)) len = sizeof(cpuFreqMHz) - 1;
    strncpy(cpuFreqMHz, p, len);
    cpuFreqMHz[len] = '\0';
    
    // Extract uptime
    p = strstr(end, "\"uptime\":\"");
    if (!p) return false;
    p += 10; // Length of "uptime":""
    end = strchr(p, '\"');
    if (!end) return false;
    len = end - p;
    if (len >= sizeof(uptime)) len = sizeof(uptime) - 1;
    strncpy(uptime, p, len);
    uptime[len] = '\0';
    
    // Extract freeHeap
    p = strstr(end, "\"freeHeap\":\"");
    if (!p) return false;
    p += 12; // Length of "freeHeap":""
    end = strchr(p, '\"');
    if (!end) return false;
    len = end - p;
    if (len >= sizeof(freeHeap)) len = sizeof(freeHeap) - 1;
    strncpy(freeHeap, p, len);
    freeHeap[len] = '\0';
    
    return true;
}
