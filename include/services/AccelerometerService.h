#ifndef ACCELEROMETER_SERVICE_H
#define ACCELEROMETER_SERVICE_H

#include "drivers/MPU6050Driver.h"
#include "models/MovementEvent.h"
#include "models/DeviceConfig.h"

/**
 * @brief Manages motion detection logic
 * 
 * Processes accelerometer data to detect significant motion events
 * with debouncing and threshold filtering.
 */
class AccelerometerService {
public:
    /**
     * @brief Construct service with hardware driver and configuration
     * @param driver Hardware abstraction for accelerometer
     * @param config Device configuration (threshold, debounce settings)
     */
    AccelerometerService(IMPU6050Driver* driver, const DeviceConfig& config);
    
    /**
     * @brief Initialize hardware and prepare for motion detection
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * @brief Check for motion and update internal state
     * @return true if significant motion detected
     */
    bool detectMotion();
    
    /**
     * @brief Get most recently detected motion event
     * @return MovementEvent with current data
     */
    MovementEvent getLastEvent() const;
    
    /**
     * @brief Calculate magnitude from 3-axis acceleration
     * @param x X-axis acceleration in G
     * @param y Y-axis acceleration in G
     * @param z Z-axis acceleration in G
     * @return Combined magnitude in G
     */
    float calculateMagnitude(float x, float y, float z) const;
    
private:
    IMPU6050Driver* mDriver;
    DeviceConfig mConfig;
    MovementEvent mLastEvent;
    unsigned long mLastMotionTime;
    bool mInMotion;
};

#endif // ACCELEROMETER_SERVICE_H
