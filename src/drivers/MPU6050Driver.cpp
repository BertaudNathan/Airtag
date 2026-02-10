#include "drivers/MPU6050Driver.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

MPU6050Driver::MPU6050Driver() 
    : mpu(nullptr)
    , mInitialized(false) {
}

bool MPU6050Driver::initialize() {
    if (mInitialized) {
        return true;
    }
    
    // Allocate MPU6050 object
    mpu = new Adafruit_MPU6050();
    if (!mpu) {
        return false;
    }
    
    Adafruit_MPU6050* mpuPtr = static_cast<Adafruit_MPU6050*>(mpu);
    
    // Attempt to initialize
    if (!mpuPtr->begin()) {
        delete mpuPtr;
        mpu = nullptr;
        return false;
    }
    
    // Configure accelerometer range (±2G for sensitivity)
    mpuPtr->setAccelerometerRange(MPU6050_RANGE_2_G);
    
    // Configure filter bandwidth
    mpuPtr->setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    mInitialized = true;
    return true;
}

bool MPU6050Driver::readAcceleration(float& x, float& y, float& z) {
    if (!mInitialized || !mpu) {
        return false;
    }
    
    Adafruit_MPU6050* mpuPtr = static_cast<Adafruit_MPU6050*>(mpu);
    sensors_event_t accel, gyro, temp;
    
    if (!mpuPtr->getEvent(&accel, &gyro, &temp)) {
        return false;
    }
    
    // Convert from m/s² to G (1G = 9.80665 m/s²)
    x = accel.acceleration.x / 9.80665f;
    y = accel.acceleration.y / 9.80665f;
    z = accel.acceleration.z / 9.80665f;
    
    return true;
}

bool MPU6050Driver::isConnected() {
    return mInitialized && (mpu != nullptr);
}
