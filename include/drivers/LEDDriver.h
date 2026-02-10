#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include <Arduino.h>

/**
 * @brief Abstract interface for LED control
 * 
 * Enables mocking for unit tests by abstracting GPIO access.
 */
class ILEDDriver {
public:
    virtual ~ILEDDriver() = default;
    
    /**
     * @brief Set LED on or off
     * @param on true to turn LED on, false to turn off
     */
    virtual void setState(bool on) = 0;
    
    /**
     * @brief Set LED brightness (if PWM capable)
     * @param level Brightness level 0-255
     */
    virtual void setBrightness(uint8_t level) = 0;
    
    /**
     * @brief Toggle LED state
     */
    virtual void toggle() = 0;
};

/**
 * @brief Concrete implementation using ESP32 GPIO
 */
class LEDDriver : public ILEDDriver {
public:
    /**
     * @brief Construct LED driver for specific GPIO pin
     * @param pin GPIO pin number
     * @param activeLow true if LED is active-low (common cathode)
     */
    LEDDriver(uint8_t pin, bool activeLow = false);
    virtual ~LEDDriver() = default;
    
    void setState(bool on) override;
    void setBrightness(uint8_t level) override;
    void toggle() override;
    
private:
    uint8_t mPin;
    bool mActiveLow;
    bool mCurrentState;
    uint8_t mPWMChannel;
};

#endif // LED_DRIVER_H
