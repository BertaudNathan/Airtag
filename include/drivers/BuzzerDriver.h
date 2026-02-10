#ifndef BUZZER_DRIVER_H
#define BUZZER_DRIVER_H

#include <Arduino.h>

/**
 * @brief Abstract interface for buzzer control
 * 
 * Enables mocking for unit tests by abstracting PWM access.
 */
class IBuzzerDriver {
public:
    virtual ~IBuzzerDriver() = default;
    
    /**
     * @brief Play tone at specified frequency
     * @param frequency Tone frequency in Hz
     * @param duration Duration in milliseconds (0 = continuous)
     */
    virtual void playTone(uint16_t frequency, uint16_t duration = 0) = 0;
    
    /**
     * @brief Stop currently playing tone
     */
    virtual void stop() = 0;
    
    /**
     * @brief Check if buzzer is currently playing
     * @return true if tone is playing
     */
    virtual bool isPlaying() = 0;
};

/**
 * @brief Concrete implementation using ESP32 PWM
 */
class BuzzerDriver : public IBuzzerDriver {
public:
    /**
     * @brief Construct buzzer driver for specific GPIO pin
     * @param pin GPIO pin number
     * @param pwmChannel PWM channel to use (0-15)
     */
    BuzzerDriver(uint8_t pin, uint8_t pwmChannel = 0);
    virtual ~BuzzerDriver();
    
    void playTone(uint16_t frequency, uint16_t duration = 0) override;
    void stop() override;
    bool isPlaying() override;
    
    /**
     * @brief Update buzzer state (call in loop for timed tones)
     */
    void update();
    
private:
    uint8_t mPin;
    uint8_t mPWMChannel;
    bool mIsPlaying;
    unsigned long mToneStartTime;
    uint16_t mToneDuration;
};

#endif // BUZZER_DRIVER_H
