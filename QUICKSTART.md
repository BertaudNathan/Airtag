# Quick Start Guide - 5 Minutes to Working System

Get your MQTT AirTag system running in 5 simple steps.

## Prerequisites Checklist

- [ ] 2x ESP32 Development Boards
- [ ] 1x MPU6050 Accelerometer module
- [ ] 1x LED (or use built-in)
- [ ] 1x Passive Buzzer
- [ ] 1x Raspberry Pi (3B+ or 4)
- [ ] 3x USB cables for power
- [ ] WiFi network (2.4GHz)
- [ ] Computer with USB ports

## Step 1: Setup Raspberry Pi Broker (2 minutes)

**On Raspberry Pi:**

```bash
# Install Mosquitto
sudo apt update && sudo apt install -y mosquitto mosquitto-clients

# Configure for network access
echo "listener 1883" | sudo tee -a /etc/mosquitto/mosquitto.conf
echo "allow_anonymous true" | sudo tee -a /etc/mosquitto/mosquitto.conf
sudo systemctl restart mosquitto

# Get IP address - WRITE THIS DOWN!
hostname -I
```

**Example output:** `192.168.1.100` ← Use this as your MQTT broker IP

## Step 2: Wire Hardware (1 minute each)

### Transmitter ESP32

```
MPU6050 → ESP32
VCC     → 3.3V
GND     → GND
SDA     → GPIO 21
SCL     → GPIO 22
```

### Receiver ESP32

```
Alert LED (+) → GPIO 4 → 220Ω resistor → GND
Buzzer (+)    → GPIO 5
Buzzer (-)    → GND
```

## Step 3: Configure Software (30 seconds each)

**Edit `src/main_transmitter.cpp` line 17:**
```cpp
strcpy(config.wifiSSID, "YourWiFiName");
strcpy(config.wifiPassword, "YourWiFiPassword");
strcpy(config.mqttBrokerIP, "192.168.1.100");  // Your Pi IP from Step 1
```

**Edit `src/main_receiver.cpp` line 18:**
```cpp
strcpy(config.wifiSSID, "YourWiFiName");
strcpy(config.wifiPassword, "YourWiFiPassword");
strcpy(config.mqttBrokerIP, "192.168.1.100");  // Your Pi IP from Step 1
```

## Step 4: Build and Upload (1 minute each)

### Install PlatformIO (one-time)

**VS Code (Recommended):**
1. Install VS Code
2. Extensions → Search "PlatformIO IDE" → Install
3. Restart VS Code

**OR Command Line:**
```bash
pip install platformio
```

### Upload to ESP32s

**In VS Code:**
1. Connect transmitter ESP32 via USB
2. PlatformIO sidebar → transmitter → Upload
3. Wait for "SUCCESS"
4. Disconnect transmitter
5. Connect receiver ESP32 via USB
6. PlatformIO sidebar → receiver → Upload
7. Wait for "SUCCESS"

**OR Command Line:**
```bash
pio run -e transmitter -t upload
# Disconnect transmitter, connect receiver
pio run -e receiver -t upload
```

## Step 5: Test! (30 seconds)

1. ✅ Both ESP32s powered on
2. ✅ Status LEDs stop blinking (means connected)
3. 🚀 **Move the transmitter device**
4. 🎉 Receiver LED blinks and buzzer sounds!

**Success!** Your AirTag system is working.

## Troubleshooting

### Status LED keeps blinking
❌ **Problem**: Not connecting to WiFi/MQTT  
✅ **Fix**: 
- Check WiFi name/password correct
- Verify Raspberry Pi IP address
- Ensure all devices on same WiFi network

### No alert when moving transmitter
❌ **Problem**: Motion not detected  
✅ **Fix**:
- Check MPU6050 wiring (especially SDA/SCL)
- Move device more vigorously
- Lower sensitivity: change `0.5f` to `0.3f` in config

### Receiver doesn't alert
❌ **Problem**: MQTT not routing messages  
✅ **Fix**:
- Check Mosquitto running: `sudo systemctl status mosquitto`
- Test broker: `mosquitto_sub -h localhost -t airtag/motion`
- Verify both ESP32s show "MQTT connected" in serial monitor

## View Serial Output

**VS Code:** PlatformIO sidebar → Environment → Monitor

**Command Line:**
```bash
pio device monitor -e transmitter
# OR
pio device monitor -e receiver
```

## Expected Serial Output

### Transmitter (on move):
```
Motion detected!
Event published to MQTT
```

### Receiver (on alert):
```
Motion event received!
Device: airtag_transmitter_01, Magnitude: 1.23
```

## Adjust Settings

### Make it more/less sensitive

In `main_transmitter.cpp`:
```cpp
config.sensitivityThreshold = 0.5f;  // Lower = more sensitive
```

Try: `0.3f` (very sensitive) or `0.8f` (less sensitive)

### Change alert duration

In `main_receiver.cpp`:
```cpp
config.alertDuration = 5000;  // Duration in milliseconds
```

Try: `3000` (3 seconds) or `10000` (10 seconds)

### Change buzzer tone

In `src/services/AlertService.cpp` line 5:
```cpp
static const uint16_t ALERT_TONE_FREQUENCY = 2000;  // Hz
```

Try: `1000` (lower pitch) or `3000` (higher pitch)

## Monitor MQTT Traffic

On Raspberry Pi:
```bash
# See all messages
mosquitto_sub -h localhost -t '#' -v

# See only motion events
mosquitto_sub -h localhost -t 'airtag/motion' -v
```

## Test Without Moving

Manually trigger alert from Raspberry Pi:
```bash
mosquitto_pub -h localhost -t 'airtag/motion' -m '{"deviceID":"test","timestamp":1000,"magnitude":1.5,"type":"start"}'
```

Receiver should alert immediately!

## Next Steps

Now that it's working:

1. **Test range**: How far can you go before WiFi drops?
2. **Test sensitivity**: What movements trigger it?
3. **Test reliability**: Leave running overnight
4. **Add more features**: See IMPLEMENTATION_SUMMARY.md for ideas

## Getting Help

Check these files for more info:
- **README.md**: Full project documentation
- **docs/raspberry-pi-setup.md**: Detailed broker setup
- **docs/build-and-flash.md**: Detailed build instructions
- **IMPLEMENTATION_SUMMARY.md**: Complete technical overview

## Success Checklist

- [x] Raspberry Pi running Mosquitto broker
- [x] Transmitter uploading and connecting
- [x] Receiver uploading and connecting
- [x] Both status LEDs solid/off (not blinking)
- [x] Motion triggers alert
- [x] Alert clearly visible and audible

**All checked?** 🎉 **Congratulations! Your MQTT AirTag system is fully operational!**

---

**Total time**: ~5 minutes  
**Difficulty**: Beginner-friendly  
**Cost**: ~$20 in parts
