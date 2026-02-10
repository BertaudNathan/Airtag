# Build and Flash Guide

This guide walks through building and uploading the AirTag firmware to ESP32 devices.

## Install PlatformIO

### Option 1: VS Code Extension (Recommended)

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Open VS Code
3. Go to Extensions (Ctrl+Shift+X)
4. Search for "PlatformIO IDE"
5. Click Install
6. Restart VS Code

### Option 2: Command Line (Advanced)

**Windows:**
```powershell
pip install platformio
```

**macOS/Linux:**
```bash
pip3 install platformio
```

Verify installation:
```bash
pio --version
```

## Open Project

### VS Code with PlatformIO Extension

1. Open VS Code
2. Click "PlatformIO" icon in sidebar
3. Click "Open Project"
4. Navigate to this folder and select it
5. PlatformIO will automatically detect `platformio.ini`

### Command Line

```bash
cd "C:\Users\Nathan Bertaud\Desktop\IOT\Airtag"
```

## Configure WiFi and MQTT

Before building, update configuration in both files:

**Edit `src/main_transmitter.cpp` line ~15:**
```cpp
void setupConfiguration() {
    strcpy(config.wifiSSID, "YourNetworkName");      // Change this
    strcpy(config.wifiPassword, "YourPassword");      // Change this
    strcpy(config.mqttBrokerIP, "192.168.1.100");    // Raspberry Pi IP
    // ... rest unchanged
}
```

**Edit `src/main_receiver.cpp` line ~16:**
```cpp
void setupConfiguration() {
    strcpy(config.wifiSSID, "YourNetworkName");      // Change this
    strcpy(config.wifiPassword, "YourPassword");      // Change this
    strcpy(config.mqttBrokerIP, "192.168.1.100");    // Raspberry Pi IP
    // ... rest unchanged
}
```

## Build Firmware

### Using VS Code PlatformIO

**Transmitter:**
1. Click PlatformIO icon in sidebar
2. Expand "transmitter" environment
3. Click "Build"

**Receiver:**
1. Click PlatformIO icon in sidebar
2. Expand "receiver" environment
3. Click "Build"

### Using Command Line

**Transmitter:**
```bash
pio run -e transmitter
```

**Receiver:**
```bash
pio run -e receiver
```

## Connect ESP32 Device

1. Connect ESP32 to computer via USB cable
2. Windows will install CH340/CP210x drivers automatically
3. Note the COM port (Device Manager → Ports)

**If drivers not installed:**
- CH340: https://sparks.gogo.co.nz/ch340.html
- CP210x: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

## Upload Firmware

### Using VS Code PlatformIO

**Transmitter:**
1. Connect first ESP32
2. Click PlatformIO icon
3. Expand "transmitter" environment
4. Click "Upload"
5. Wait for "SUCCESS" message

**Receiver:**
1. Disconnect transmitter, connect second ESP32
2. Click PlatformIO icon
3. Expand "receiver" environment
4. Click "Upload"
5. Wait for "SUCCESS" message

### Using Command Line

**Transmitter:**
```bash
pio run -e transmitter -t upload
```

**Receiver:**
```bash
pio run -e receiver -t upload
```

**Specify COM port manually (if needed):**
```bash
pio run -e transmitter -t upload --upload-port COM3
pio run -e receiver -t upload --upload-port COM4
```

## Monitor Serial Output

### Using VS Code PlatformIO

1. Click PlatformIO icon
2. Expand environment (transmitter or receiver)
3. Click "Monitor"

### Using Command Line

**Transmitter:**
```bash
pio device monitor -e transmitter
```

**Receiver:**
```bash
pio device monitor -e receiver
```

**Exit monitor:** Press `Ctrl+C`

## Expected Serial Output

### Transmitter (on startup)

```
=== AirTag Transmitter Starting ===
Accelerometer initialized
Connecting to WiFi...
WiFi connected
Connecting to MQTT broker...
MQTT connected
Setup complete - monitoring for motion
```

### Receiver (on startup)

```
=== AirTag Receiver Starting ===
Hardware initialized
Connecting to WiFi...
WiFi connected
Connecting to MQTT broker...
MQTT connected
Subscribed to motion topic
Setup complete - listening for motion events
```

### Transmitter (when moved)

```
Motion detected!
Event published to MQTT
```

### Receiver (when motion received)

```
Motion event received!
Device: airtag_transmitter_01, Magnitude: 1.23
```

## Troubleshooting Builds

### Error: "Platform 'espressif32' not installed"

**Solution:**
```bash
pio platform install espressif32
```

### Error: "Library X not found"

**Solution:** PlatformIO will auto-install on first build. If issues:
```bash
pio lib install "knolleary/PubSubClient"
pio lib install "adafruit/Adafruit MPU6050"
```

### Error: "Upload Port not found"

**Solution:**
1. Check USB cable is data-capable (not charge-only)
2. Install drivers (see "Connect ESP32 Device" section)
3. Try different USB port
4. Manually specify port: `--upload-port COM3`

### Error: "Timed out waiting for packet header"

**Solution:**
1. Hold "BOOT" button on ESP32
2. Click Upload in PlatformIO
3. Release "BOOT" when "Connecting..." appears

### Build Warnings

PlatformIO may show warnings - these are usually safe to ignore unless build fails.

## Clean Build (if issues)

```bash
pio run -e transmitter -t clean
pio run -e receiver -t clean
```

Then rebuild.

## Update Dependencies

```bash
pio pkg update
```

## Pin Configuration Summary

### Transmitter

| Component | Pin | Notes |
|-----------|-----|-------|
| Status LED | GPIO 2 | Built-in LED |
| MPU6050 SDA | GPIO 21 | I2C Data |
| MPU6050 SCL | GPIO 22 | I2C Clock |

### Receiver

| Component | Pin | Notes |
|-----------|-----|-------|
| Status LED | GPIO 2 | Built-in LED |
| Alert LED | GPIO 4 | External LED + resistor |
| Buzzer | GPIO 5 | Passive buzzer |

**Change pins:** Edit the `#define` statements at the top of main files.

## Next Steps After Upload

1. ✅ Transmitter and receiver uploaded
2. ✅ Both showing "Setup complete" in serial monitor
3. → Move transmitter device
4. → Verify receiver LED blinks and buzzer sounds
5. → Check Raspberry Pi MQTT broker logs

## Common Issues

### "Setup complete" but nothing happens

1. Check Raspberry Pi MQTT broker is running
2. Verify WiFi credentials are correct
3. Check Raspberry Pi IP address
4. Monitor MQTT: `mosquitto_sub -h localhost -t 'airtag/motion'`

### Transmitter detects motion but receiver doesn't respond

1. Verify both are connected (status LED off = connected)
2. Check MQTT broker receives messages
3. Confirm both use same MQTT topic
4. Monitor receiver serial output

### Constant motion detection (false positives)

1. Increase sensitivity threshold (e.g., 0.8 instead of 0.5)
2. Ensure accelerometer is firmly mounted
3. Check for environmental vibration

## Performance Testing

### Latency Test

1. Connect both devices to serial monitors side-by-side
2. Move transmitter
3. Measure time between "Motion detected!" and receiver alert

**Expected:** <500ms transmitter, <200ms receiver alert

### Stability Test

Leave system running for 24 hours:
- Monitor for crashes
- Check for memory leaks
- Verify automatic reconnection after network dropout

## Advanced: OTA Updates (Future)

Over-the-Air (OTA) updates not implemented in MVP. For firmware updates:
1. Connect USB cable
2. Re-upload via PlatformIO

## Build Artifacts

After successful build, compiled firmware is at:
- Transmitter: `.pio/build/transmitter/firmware.bin`
- Receiver: `.pio/build/receiver/firmware.bin`

These can be flashed using `esptool.py` if needed.

## Support

**Build issues:** Check PlatformIO documentation at docs.platformio.org  
**Hardware issues:** Check serial monitor output for error messages  
**MQTT issues:** See `docs/raspberry-pi-setup.md`
