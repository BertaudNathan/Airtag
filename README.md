# MQTT AirTag Tracking System

An ESP32-based IoT motion tracking system using MQTT for communication. When the transmitter device detects movement via accelerometer, it alerts receiver devices through LED and buzzer notifications.

## System Architecture

### Components

1. **Transmitter ESP32**: Detects motion using MPU6050 accelerometer and publishes alerts via MQTT
2. **Receiver ESP32**: Subscribes to MQTT alerts and activates LED + buzzer notifications
3. **Raspberry Pi**: Hosts Mosquitto MQTT broker for message routing

All devices connect to the same WiFi network.

### Features

- ✅ Real-time motion detection with configurable sensitivity
- ✅ MQTT-based publish/subscribe architecture
- ✅ Automatic WiFi and MQTT reconnection with exponential backoff
- ✅ Audio-visual alerts (LED + buzzer)
- ✅ Debouncing to prevent false positives
- ✅ Constitution-compliant code (clean architecture, TDD-ready, ≤30 lines/function)

## Project Structure

```
include/          # Header files (.h)
├── drivers/      # Hardware abstraction (MPU6050, LED, Buzzer)
├── models/       # Data structures (MovementEvent, DeviceConfig, etc.)
├── services/     # Business logic (Accelerometer, MQTT, WiFi, Alert)
└── utils/        # Utilities (Logger)

src/              # Implementation files (.cpp)
├── drivers/
├── models/
├── services/
├── utils/
├── main_transmitter.cpp    # Transmitter firmware
└── main_receiver.cpp       # Receiver firmware

docs/
└── raspberry-pi-setup.md   # MQTT broker setup guide

specs/001-mqtt-airtag/
├── spec.md                 # Feature specification
├── plan.md                 # Implementation plan
└── checklists/
    └── requirements.md     # Quality validation
```

## Hardware Requirements

### Transmitter

- ESP32 Development Board
- MPU6050 Accelerometer (I2C)
- Status LED (or use built-in LED on GPIO 2)
- USB power supply or battery

**Wiring:**
- MPU6050 SDA → ESP32 GPIO 21
- MPU6050 SCL → ESP32 GPIO 22
- MPU6050 VCC → 3.3V
- MPU6050 GND → GND

### Receiver

- ESP32 Development Board
- Status LED (GPIO 2)
- Alert LED (GPIO 4)
- Passive buzzer (GPIO 5)
- USB power supply

**Wiring:**
- Alert LED (+) → ESP32 GPIO 4 → Resistor (220Ω) → GND
- Buzzer (+) → ESP32 GPIO 5
- Buzzer (-) → GND

### Raspberry Pi Broker

- Raspberry Pi 4 (or Pi 3 B+)
- MicroSD card with Raspberry Pi OS
- Power supply
- Network connection (WiFi or Ethernet)

## Quick Start

### 1. Setup Raspberry Pi MQTT Broker

Follow the detailed guide: [docs/raspberry-pi-setup.md](docs/raspberry-pi-setup.md)

Quick commands:
```bash
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
echo "listener 1883" | sudo tee -a /etc/mosquitto/mosquitto.conf
echo "allow_anonymous true" | sudo tee -a /etc/mosquitto/mosquitto.conf
sudo systemctl restart mosquitto
```

Get Raspberry Pi IP:
```bash
hostname -I
```

### 2. Configure ESP32 Devices

Edit both `src/main_transmitter.cpp` and `src/main_receiver.cpp`:

```cpp
void setupConfiguration() {
    strcpy(config.wifiSSID, "YOUR_WIFI_SSID");        // Your WiFi network name
    strcpy(config.wifiPassword, "YOUR_WIFI_PASSWORD"); // Your WiFi password
    strcpy(config.mqttBrokerIP, "192.168.1.100");     // Your Raspberry Pi IP
    // ... rest remains the same
}
```

### 3. Build and Upload Firmware

**Install PlatformIO CLI** (if not installed):
```bash
pip install platformio
```

**Build Transmitter:**
```bash
pio run -e transmitter
pio run -e transmitter -t upload
```

**Build Receiver:**
```bash
pio run -e receiver
pio run -e receiver -t upload
```

### 4. Monitor Serial Output

**Transmitter:**
```bash
pio device monitor -e transmitter
```

**Receiver:**
```bash
pio device monitor -e receiver
```

### 5. Test the System

1. Power on Raspberry Pi and verify Mosquitto is running
2. Upload transmitter firmware to first ESP32
3. Upload receiver firmware to second ESP32
4. Move the transmitter device - receiver should alert!

## Configuration Options

### Motion Sensitivity

In `setupConfiguration()`:
```cpp
config.sensitivityThreshold = 0.5f;  // G-force threshold (default: 0.5G)
```

Lower values = more sensitive to gentle movements  
Higher values = only detect stronger movements

### Alert Duration

```cpp
config.alertDuration = 5000;  // Duration in milliseconds (default: 5 seconds)
```

### Debounce Window

```cpp
config.debounceWindow = 1000;  // Milliseconds (default: 1 second)
```

Prevents rapid repeated alerts during continuous motion.

### MQTT Topic

```cpp
strcpy(config.mqttTopic, "airtag/motion");  // Change to your preferred topic
```

## LED Status Indicators

### Transmitter

- **Off**: Fully connected and idle
- **Slow blink**: WiFi/MQTT connection issue (retrying)
- **Quick flash**: Motion detected and published

### Receiver

- **Off**: Fully connected and listening
- **Slow blink**: WiFi/MQTT connection issue (retrying)
- **Fast blink + buzzer**: Motion alert active

## Monitoring

### Monitor MQTT Traffic

On Raspberry Pi:
```bash
# View all messages
mosquitto_sub -h localhost -t '#' -v

# View only motion events
mosquitto_sub -h localhost -t 'airtag/motion' -v
```

### Test MQTT Manually

Publish test event:
```bash
mosquitto_pub -h localhost -t 'airtag/motion' -m '{"deviceID":"test","timestamp":1000,"magnitude":1.5,"type":"start"}'
```

Receiver should trigger alert.

## Troubleshooting

### ESP32 Can't Connect to WiFi

- Verify SSID/password are correct
- Check WiFi signal strength (need -75 dBm or better)
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check Serial monitor for error messages

### ESP32 Can't Connect to MQTT

- Verify Raspberry Pi IP address
- Ensure Mosquitto is running: `sudo systemctl status mosquitto`
- Check firewall: `sudo ufw allow 1883/tcp`
- Verify ESP32 and Pi are on same network
- Test with: `mosquitto_sub -h <PI_IP> -t test`

### Accelerometer Not Detected

- Check I2C wiring (SDA/SCL, VCC/GND)
- Verify MPU6050 address (default 0x68)
- Test with I2C scanner sketch
- Check Serial monitor for "Failed to initialize accelerometer"

### No Motion Detection

- Increase sensitivity: lower `sensitivityThreshold` value
- Check Serial monitor for accelerometer readings
- Verify MPU6050 is securely mounted (not loose)

### Receiver Not Alerting

- Verify receiver subscribed successfully (check Serial)
- Test with manual MQTT publish (see above)
- Check LED/buzzer wiring
- Monitor MQTT broker to see if messages arrive

## Development

### Constitution Compliance

This project follows strict code quality principles:

- ✅ Clean architecture (.h/.cpp separation)
- ✅ Functions ≤30 lines and ≤5 nesting depth
- ✅ Classes for stateful components, structs for data
- ✅ Test-driven development ready (mocked interfaces)
- ✅ RAII and const-correctness

See [.specify/memory/constitution.md](.specify/memory/constitution.md) for details.

### Testing

Unit tests use mocked hardware interfaces:

```cpp
class MockMPU6050 : public IMPU6050Driver {
    // Test implementation
};
```

Run tests:
```bash
pio test
```

### Adding Features

1. Review [specs/001-mqtt-airtag/spec.md](specs/001-mqtt-airtag/spec.md)
2. Follow TDD: write tests first
3. Implement feature following constitution
4. Verify all functions meet complexity limits

## Performance

- **Motion-to-MQTT latency**: <500ms typical
- **MQTT-to-alert latency**: <200ms typical
- **WiFi reconnection**: ~5-30 seconds (exponential backoff)
- **Memory usage**: ~50KB RAM (plenty of headroom)
- **Sampling rate**: 20Hz (50ms interval)

## Security Notes

**⚠️ Current configuration is for development only:**

- MQTT has no authentication (anonymous allowed)
- WiFi credentials stored in code (plaintext)
- No encryption (TLS) for MQTT

**For production:**
- Enable Mosquitto authentication
- Use secrets management for credentials
- Enable MQTT TLS/SSL
- Implement device attestation

## License

This project is built for educational and personal use.

## Contributing

Follow the project constitution when contributing:
- Write tests first (TDD)
- Keep functions small (≤30 lines)
- Separate headers (.h) and implementations (.cpp)
- Document public APIs

## Support

For issues:
1. Check Serial monitor logs
2. Review troubleshooting section
3. Check Mosquitto logs: `sudo tail -f /var/log/mosquitto/mosquitto.log`
4. Verify network connectivity

## Version

**Current**: 1.0.0 (MVP - Motion Detection & Alert System)

**Branch**: `001-mqtt-airtag`
