#include "services/AlertService.h"

static const uint16_t ALERT_TONE_FREQUENCY = 2000;  // 2kHz tone
static const uint16_t BLINK_INTERVAL = 250;         // 250ms blink

AlertService::AlertService(ILEDDriver* led, IBuzzerDriver* buzzer)
    : mLED(led)
    , mBuzzer(buzzer)
    , mState()
    , mLastBlinkTime(0)
    , mBlinkState(false) {
}

void AlertService::triggerAlert(uint32_t duration, const MovementEvent& event) {
    mState.startAlert(duration, event);
    activateOutputs();
}

void AlertService::update() {
    unsigned long currentTime = millis();
    
    if (!mState.shouldBeActive(currentTime)) {
        if (mState.isAlerting) {
            stopAlert();
        }
        return;
    }
    
    // Update blinking LED pattern
    updateBlinkPattern();
}

void AlertService::stopAlert() {
    mState.stopAlert();
    deactivateOutputs();
}

bool AlertService::isAlerting() const {
    return mState.isAlerting;
}

void AlertService::activateOutputs() {
    if (mLED) {
        mLED->setState(true);
        mBlinkState = true;
        mLastBlinkTime = millis();
    }
    
    if (mBuzzer) {
        mBuzzer->playTone(ALERT_TONE_FREQUENCY);
    }
}

void AlertService::deactivateOutputs() {
    if (mLED) {
        mLED->setState(false);
    }
    
    if (mBuzzer) {
        mBuzzer->stop();
    }
}

void AlertService::updateBlinkPattern() {
    if (!mLED) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    if (currentTime - mLastBlinkTime >= BLINK_INTERVAL) {
        mBlinkState = !mBlinkState;
        mLED->setState(mBlinkState);
        mLastBlinkTime = currentTime;
    }
}
