#include "models/MovementEvent.h"
#include <string.h>
#include <stdio.h>

MovementEvent::MovementEvent() 
    : timestamp(0)
    , accelerationMagnitude(0.0f)
    , type(MOTION_START) {
    memset(deviceID, 0, sizeof(deviceID));
}

MovementEvent::MovementEvent(const char* id, unsigned long ts, 
                             float magnitude, EventType eventType)
    : timestamp(ts)
    , accelerationMagnitude(magnitude)
    , type(eventType) {
    strncpy(deviceID, id, sizeof(deviceID) - 1);
    deviceID[sizeof(deviceID) - 1] = '\0';
}

bool MovementEvent::toJSON(char* buffer, size_t bufferSize) const {
    if (!buffer || bufferSize < 128) {
        return false;
    }
    
    const char* typeStr = "unknown";
    switch (type) {
        case MOTION_START: typeStr = "start"; break;
        case MOTION_CONTINUE: typeStr = "continue"; break;
        case MOTION_STOP: typeStr = "stop"; break;
    }
    
    int written = snprintf(buffer, bufferSize,
        "{\"deviceID\":\"%s\",\"timestamp\":%lu,\"magnitude\":%.2f,\"type\":\"%s\"}",
        deviceID, timestamp, accelerationMagnitude, typeStr);
    
    return written > 0 && written < (int)bufferSize;
}

bool MovementEvent::fromJSON(const char* jsonStr) {
    if (!jsonStr) {
        return false;
    }
    
    // Simple JSON parsing (limited but adequate for known format)
    char typeStr[16] = {0};
    int matched = sscanf(jsonStr,
        "{\"deviceID\":\"%31[^\"]\",\"timestamp\":%lu,\"magnitude\":%f,\"type\":\"%15[^\"]\"}",
        deviceID, &timestamp, &accelerationMagnitude, typeStr);
    
    if (matched != 4) {
        return false;
    }
    
    if (strcmp(typeStr, "start") == 0) {
        type = MOTION_START;
    } else if (strcmp(typeStr, "continue") == 0) {
        type = MOTION_CONTINUE;
    } else if (strcmp(typeStr, "stop") == 0) {
        type = MOTION_STOP;
    } else {
        return false;
    }
    
    return true;
}
