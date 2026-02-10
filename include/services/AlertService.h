#ifndef ALERT_SERVICE_H
#define ALERT_SERVICE_H

#include "drivers/LEDDriver.h"
#include "drivers/BuzzerDriver.h"
#include "models/AlertState.h"
#include "models/MovementEvent.h"

/**
 * @brief Manages audio-visual alerts
 * 
 * Controls LED and buzzer hardware to provide user notifications
 * for motion detection events.
 */
class AlertService {
public:
    /**
     * @brief Construct service with hardware drivers
     * @param led LED driver for visual feedback
     * @param buzzer Buzzer driver for audio feedback
     */
    AlertService(ILEDDriver* led, IBuzzerDriver* buzzer);
    
    /**
     * @brief Trigger alert for specified duration
     * @param duration Alert duration in milliseconds
     * @param event Event that triggered the alert
     */
    void triggerAlert(uint32_t duration, const MovementEvent& event);
    
    /**
     * @brief Update alert state (call in loop)
     * 
     * Manages alert timing and automatically stops when expired.
     */
    void update();
    
    /**
     * @brief Stop alert immediately
     */
    void stopAlert();
    
    /**
     * @brief Check if currently alerting
     * @return true if alert is active
     */
    bool isAlerting() const;
    
private:
    ILEDDriver* mLED;
    IBuzzerDriver* mBuzzer;
    AlertState mState;
    unsigned long mLastBlinkTime;
    bool mBlinkState;
    
    void activateOutputs();
    void deactivateOutputs();
    void updateBlinkPattern();
};

#endif // ALERT_SERVICE_H
