#ifndef MPU6050_DRIVER_H
#define MPU6050_DRIVER_H

#include <Arduino.h>

/**
 * @brief Abstract interface for MPU6050 accelerometer
 * 
 * Enables mocking for unit tests by abstracting hardware access.
 */
class IMPU6050Driver {
public:
    virtual ~IMPU6050Driver() = default;
    
    /**
     * @brief Initialize accelerometer hardware
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Read current acceleration values
     * @param x Output for X-axis acceleration in G
     * @param y Output for Y-axis acceleration in G
     * @param z Output for Z-axis acceleration in G
     * @return true if read successful
     */
    virtual bool readAcceleration(float& x, float& y, float& z) = 0;
    
    /**
     * @brief Check if accelerometer is responsive
     * @return true if device responding correctly
     */
    virtual bool isConnected() = 0;
};

/**
 * @brief Concrete implementation using Adafruit MPU6050 library
 */
class MPU6050Driver : public IMPU6050Driver {
public:
    MPU6050Driver();
    virtual ~MPU6050Driver() = default;
    
    bool initialize() override;
    bool readAcceleration(float& x, float& y, float& z) override;
    bool isConnected() override;
    
private:
    void* mpu;  // Opaque pointer to avoid header dependency
    bool mInitialized;
};

#endif // MPU6050_DRIVER_H
