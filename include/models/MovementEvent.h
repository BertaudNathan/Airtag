#ifndef MOVEMENT_EVENT_H
#define MOVEMENT_EVENT_H

#include <Arduino.h>
#include "interfaces/IEvent.h"

/**
 * @brief Represents a detected motion event
 * 
 * This struct contains all data associated with a single motion detection
 * event, including device identification, timing, and acceleration magnitude.
 */
struct MovementEvent : IEvent {
    enum EventType {
        MOTION_START,      // Initial motion detected
        MOTION_CONTINUE,   // Ongoing motion
        MOTION_STOP        // Motion ceased
    };
    
    char deviceID[32];             // Unique device identifier
    unsigned long timestamp;        // Millis since boot
    float accelerationMagnitude;    // Combined X/Y/Z magnitude in G
    EventType type;                 // Event classification
    
    /**
     * @brief Default constructor initializes to safe defaults
     */
    MovementEvent();
    
    /**
     * @brief Parameterized constructor
     */
    MovementEvent(const char* id, unsigned long ts, float magnitude, EventType eventType);
    
    /**
     * @brief Serialize event to JSON string
     * @param buffer Output buffer for JSON string
     * @param bufferSize Size of output buffer
     * @return true if serialization successful
     */
    bool toJSON(char* buffer, size_t bufferSize) const override;
    
    /**
     * @brief Deserialize event from JSON string
     * @param jsonStr Input JSON string
     * @return true if deserialization successful
     */
    bool fromJSON(const char* jsonStr) override;
    
    /**
     * @brief Get the event type identifier
     * @return EventTypeID for this event
     */
    EventTypeID getEventTypeID() const override { return EventTypeID::MOVEMENT_EVENT; }
};

#endif // MOVEMENT_EVENT_H