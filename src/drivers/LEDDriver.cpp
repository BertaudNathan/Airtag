#include "drivers/LEDDriver.h"

LEDDriver::LEDDriver(uint8_t pin, bool activeLow)
    : mPin(pin)
    , mActiveLow(activeLow)
    , mCurrentState(false)
    , mPWMChannel(255) {  // Invalid channel initially
    
    pinMode(mPin, OUTPUT);
    setState(false);
}

void LEDDriver::setState(bool on) {
    mCurrentState = on;
    
    // Apply active-low logic if needed
    bool physicalState = mActiveLow ? !on : on;
    
    digitalWrite(mPin, physicalState ? HIGH : LOW);
}

void LEDDriver::setBrightness(uint8_t level) {
    // Setup PWM channel if not already configured
    if (mPWMChannel == 255) {
        // Use first available channel (could be improved with channel management)
        mPWMChannel = 0;
        ledcSetup(mPWMChannel, 5000, 8);  // 5kHz, 8-bit resolution
        ledcAttachPin(mPin, mPWMChannel);
    }
    
    // Apply active-low logic
    uint8_t dutyCycle = mActiveLow ? (255 - level) : level;
    
    ledcWrite(mPWMChannel, dutyCycle);
    mCurrentState = (level > 0);
}

void LEDDriver::toggle() {
    setState(!mCurrentState);
}
