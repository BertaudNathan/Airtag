#ifndef IEVENT_H
#define IEVENT_H
#include <Arduino.h>
/**
 * @brief Base interface for all event types
 * 
 * This interface can be extended to create specific event types (e.g. MovementEvent).
 * It allows for polymorphic handling of different events in services like MQTTService.
 */
class IEvent {
public:
    virtual ~IEvent() {}
    bool toJSON(char* buffer, size_t bufferSize) const;
    
    /**
     * @brief Deserialize event from JSON string
     * @param jsonStr Input JSON string
     * @return true if deserialization successful
     */
    bool fromJSON(const char* jsonStr);
};

#endif // IEVENT_H