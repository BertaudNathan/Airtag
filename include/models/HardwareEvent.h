# ifndef HARDWARE_EVENT_H
#define HARDWARE_EVENT_H
#include <Arduino.h>
#include "interfaces/IEvent.h"
/**
 * @brief Represents a hardware status event
 * 
 * This struct contains data about the device's hardware status, such as RAM usage,
 * CPU frequency, uptime, and free heap memory. It can be published to MQTT for monitoring.
 */

struct HardwareEvent : IEvent {
    char deviceID[32];             // Unique device identifier
    unsigned long timestamp;        // Millis since boot
    char ramUsage[16];             // RAM usage percentage as string
    char cpuFreqMHz[16];           // CPU frequency in MHz as string
    char uptime[32];               // Uptime in seconds as string
    char freeHeap[16];             // Free heap memory in bytes as string
    
    /**
     * @brief Default constructor initializes to safe defaults
     */
    HardwareEvent();
    
    /**
     * @brief Parameterized constructor
     */
    HardwareEvent(const char* id, unsigned long ts, const char* ram, const char* cpu, const char* uptimeStr, const char* heap);
    
    /**
     * @brief Serialize event to JSON string
     * @param buffer Output buffer for JSON string
     * @param bufferSize Size of output buffer
     * @return true if serialization successful
     */
    bool toJSON(char* buffer, size_t bufferSize) const;
    
    /**
     * @brief Deserialize event from JSON string
     * @param jsonStr Input JSON string
     * @return true if deserialization successful
     */
    bool fromJSON(const char* jsonStr);
};

#endif // HARDWARE_EVENT_H