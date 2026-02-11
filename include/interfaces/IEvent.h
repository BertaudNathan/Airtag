#ifndef IEVENT_H
#define IEVENT_H
#include <Arduino.h>

/**
 * @brief Event type identifiers
 */
enum class EventTypeID {
    MOVEMENT_EVENT,
    HARDWARE_EVENT
};

/**
 * @brief Base interface for all event types
 * 
 * This interface can be extended to create specific event types (e.g. MovementEvent).
 * It allows for polymorphic handling of different events in services like MQTTService.
 */
class IEvent {
public:
    virtual ~IEvent() {}
    virtual bool toJSON(char* buffer, size_t bufferSize) const = 0;
    
    /**
     * @brief Deserialize event from JSON string
     * @param jsonStr Input JSON string
     * @return true if deserialization successful
     */
    virtual bool fromJSON(const char* jsonStr) = 0;
    
    /**
     * @brief Get the event type identifier
     * @return EventTypeID for this event
     */
    virtual EventTypeID getEventTypeID() const = 0;
};

#endif // IEVENT_H