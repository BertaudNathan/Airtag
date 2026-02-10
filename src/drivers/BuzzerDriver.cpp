#include "drivers/BuzzerDriver.h"

BuzzerDriver::BuzzerDriver(uint8_t pin, uint8_t pwmChannel)
    : mPin(pin)
    , mPWMChannel(pwmChannel)
    , mIsPlaying(false)
    , mToneStartTime(0)
    , mToneDuration(0) {
    
    pinMode(mPin, OUTPUT);
    
    // Setup PWM channel for tone generation
    ledcSetup(mPWMChannel, 2000, 8);  // Initial freq, 8-bit resolution
    ledcAttachPin(mPin, mPWMChannel);
    ledcWrite(mPWMChannel, 0);  // Start silent
}

BuzzerDriver::~BuzzerDriver() {
    stop();
    ledcDetachPin(mPin);
}

void BuzzerDriver::playTone(uint16_t frequency, uint16_t duration) {
    if (frequency == 0) {
        stop();
        return;
    }
    
    // Configure PWM frequency for the tone
    ledcSetup(mPWMChannel, frequency, 8);
    ledcAttachPin(mPin, mPWMChannel);
    
    // 50% duty cycle for square wave tone
    ledcWrite(mPWMChannel, 128);
    
    mIsPlaying = true;
    mToneStartTime = millis();
    mToneDuration = duration;
}

void BuzzerDriver::stop() {
    ledcWrite(mPWMChannel, 0);
    mIsPlaying = false;
    mToneDuration = 0;
}

bool BuzzerDriver::isPlaying() {
    return mIsPlaying;
}

void BuzzerDriver::update() {
    // Stop tone if duration expired
    if (mIsPlaying && mToneDuration > 0) {
        if (millis() - mToneStartTime >= mToneDuration) {
            stop();
        }
    }
}
