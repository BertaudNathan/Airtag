#ifndef ALERT_STATE_H
#define ALERT_STATE_H

#include <Arduino.h>
#include "MovementEvent.h"

/**
 * @brief Tracks current alert status on receiver device
 * 
 * Manages timing and state for audio-visual alerts triggered by
 * movement detection events.
 */
struct AlertState {
    bool isAlerting;                // Currently in alert mode
    unsigned long alertStartTime;   // When alert began (millis)
    unsigned long alertEndTime;     // When alert should end (millis)
    MovementEvent lastEvent;        // Most recent triggering event
    
    /**
     * @brief Default constructor initializes to inactive state
     */
    AlertState();
    
    /**
     * @brief Check if alert should still be active
     * @param currentTime Current time in milliseconds
     * @return true if alert is active and not expired
     */
    bool shouldBeActive(unsigned long currentTime) const;
    
    /**
     * @brief Start new alert with specified duration
     * @param duration Alert duration in milliseconds
     * @param event Event that triggered the alert
     */
    void startAlert(uint32_t duration, const MovementEvent& event);
    
    /**
     * @brief Extend alert duration (for rapid multiple events)
     * @param additionalTime Additional milliseconds to extend
     */
    void extendAlert(uint32_t additionalTime);
    
    /**
     * @brief Stop alert immediately
     */
    void stopAlert();
};

#endif // ALERT_STATE_H
