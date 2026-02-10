#include "models/AlertState.h"

AlertState::AlertState() 
    : isAlerting(false)
    , alertStartTime(0)
    , alertEndTime(0)
    , lastEvent() {
}

bool AlertState::shouldBeActive(unsigned long currentTime) const {
    return isAlerting && (currentTime < alertEndTime);
}

void AlertState::startAlert(uint32_t duration, const MovementEvent& event) {
    isAlerting = true;
    alertStartTime = millis();
    alertEndTime = alertStartTime + duration;
    lastEvent = event;
}

void AlertState::extendAlert(uint32_t additionalTime) {
    if (isAlerting) {
        alertEndTime += additionalTime;
    }
}

void AlertState::stopAlert() {
    isAlerting = false;
    alertStartTime = 0;
    alertEndTime = 0;
}
