#include "services/AccelerometerService.h"
#include <math.h>

AccelerometerService::AccelerometerService(IMPU6050Driver* driver, 
                                           const DeviceConfig& config)
    : mDriver(driver)
    , mConfig(config)
    , mLastEvent()
    , mLastMotionTime(0)
    , mInMotion(false) {
}

bool AccelerometerService::begin() {
    if (!mDriver) {
        return false;
    }
    
    return mDriver->initialize();
}

bool AccelerometerService::detectMotion() {
    if (!mDriver || !mDriver->isConnected()) {
        return false;
    }
    
    float x, y, z;
    if (!mDriver->readAcceleration(x, y, z)) {
        return false;
    }
    
    float magnitude = calculateMagnitude(x, y, z);
    unsigned long currentTime = millis();
    
    // Check if magnitude exceeds threshold
    bool motionDetected = (magnitude > mConfig.sensitivityThreshold);
    
    // Implement debouncing
    if (motionDetected) {
        if (!mInMotion) {
            // New motion started
            mLastEvent = MovementEvent(mConfig.deviceID, currentTime, 
                                       magnitude, MovementEvent::MOTION_START);
            mInMotion = true;
        } else {
            // Ongoing motion
            mLastEvent = MovementEvent(mConfig.deviceID, currentTime, 
                                       magnitude, MovementEvent::MOTION_CONTINUE);
        }
        mLastMotionTime = currentTime;
        return true;
    } else {
        // No motion currently
        if (mInMotion && (currentTime - mLastMotionTime > mConfig.debounceWindow)) {
            // Motion stopped
            mLastEvent = MovementEvent(mConfig.deviceID, currentTime, 
                                       magnitude, MovementEvent::MOTION_STOP);
            mInMotion = false;
            return true;  // Report stop event
        }
    }
    
    return false;
}

MovementEvent AccelerometerService::getLastEvent() const {
    return mLastEvent;
}

float AccelerometerService::calculateMagnitude(float x, float y, float z) const {
    // Euclidean distance in 3D space
    return sqrt(x * x + y * y + z * z);
}
