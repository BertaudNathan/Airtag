# Implementation Plan: MQTT-Based AirTag Tracking System

**Branch**: `001-mqtt-airtag` | **Date**: 2026-02-10 | **Spec**: [spec.md](spec.md)

## Summary

Build an IoT motion tracking system with two ESP32 devices: a transmitter that detects movement via accelerometer and publishes alerts to MQTT, and a receiver that subscribes to alerts and activates LED/buzzer notifications. Communication facilitated by Mosquitto MQTT broker on Raspberry Pi. All devices connect to common WiFi network.

## Technical Context

**Language/Version**: C++11 (Arduino framework for ESP32)  
**Primary Dependencies**: Arduino, WiFi (ESP32 core), PubSubClient (MQTT), Wire (I2C for accelerometer)  
**Storage**: N/A (no persistent storage in MVP)  
**Testing**: PlatformIO Unit Testing framework with mocking for hardware interfaces  
**Target Platform**: ESP32 DevKit (both transmitter and receiver), Raspberry Pi 4 (MQTT broker)  
**Project Type**: Embedded/IoT - dual firmware variants (transmitter/receiver)  
**Performance Goals**: <500ms motion-to-publish latency, <200ms MQTT-to-alert latency, 24hr+ stable operation  
**Constraints**: Limited RAM (520KB), single-core execution, WiFi range dependent, no floating-point optimization  
**Scale/Scope**: 2 ESP32 devices, 1 MQTT broker, ~5-10 messages/minute typical load, indoor WiFi range

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Verify compliance with `.specify/memory/constitution.md`:

- [x] **Clean Code Architecture**: Plan includes .h/.cpp split strategy (headers in `include/`, implementations in `src/`)
- [x] **TDD Strategy**: Red-Green-Refactor workflow defined (write tests first, mock hardware interfaces)
- [x] **Function Complexity**: Design accounts for 30-line, 5-depth limits (small focused functions for sensor reading, MQTT ops, alert control)
- [x] **Embedded Best Practices**: Memory and resource management strategy defined (RAII for connections, const-correctness, stack allocation)
- [x] **Quality Gates**: Review and testing checkpoints identified (pre-merge constitution compliance check)

**Violations/Justifications**: None - architecture designed to comply with all principles

## Project Structure

### Documentation (this feature)

```text
specs/001-mqtt-airtag/
├── plan.md              # This file
├── spec.md              # Feature specification
├── checklists/
│   └── requirements.md  # Spec quality validation
└── tasks.md             # Implementation tasks (created by /speckit.tasks)
```

### Source Code (repository root)

```text
# Option 4: Embedded/IoT (ESP32/Arduino)
include/
├── models/
│   ├── MovementEvent.h       # Motion event data structure
│   ├── DeviceConfig.h        # Configuration parameters
│   ├── AlertState.h          # Alert status tracking
│   └── ConnectionStatus.h    # Network status tracking
├── services/
│   ├── AccelerometerService.h    # Motion detection logic
│   ├── MQTTService.h             # MQTT pub/sub operations
│   ├── WiFiService.h             # WiFi connection management
│   └── AlertService.h            # LED/buzzer control
├── drivers/
│   ├── MPU6050Driver.h           # Accelerometer hardware abstraction
│   ├── LEDDriver.h               # LED control abstraction
│   └── BuzzerDriver.h            # Buzzer control abstraction
└── utils/
    ├── Logger.h                  # Serial logging utility
    └── ConnectionRetry.h         # Exponential backoff logic

src/
├── models/
│   ├── MovementEvent.cpp
│   ├── DeviceConfig.cpp
│   ├── AlertState.cpp
│   └── ConnectionStatus.cpp
├── services/
│   ├── AccelerometerService.cpp
│   ├── MQTTService.cpp
│   ├── WiFiService.cpp
│   └── AlertService.cpp
├── drivers/
│   ├── MPU6050Driver.cpp
│   ├── LEDDriver.cpp
│   └── BuzzerDriver.cpp
├── utils/
│   ├── Logger.cpp
│   └── ConnectionRetry.cpp
├── main_transmitter.cpp          # Transmitter entry point
└── main_receiver.cpp             # Receiver entry point

lib/                              # External libraries (PubSubClient, MPU6050 lib)

test/
├── test_accelerometer/           # AccelerometerService unit tests
├── test_mqtt/                    # MQTTService unit tests  
├── test_alert/                   # AlertService unit tests
└── test_integration/             # End-to-end hardware mocking tests

docs/
└── raspberry-pi-setup.md         # Mosquitto broker installation guide
```

**Structure Decision**: Using embedded/IoT structure (Option 4) with strict .h/.cpp separation per constitution. Separate main files for transmitter and receiver firmware variants built from shared source base.

## Complexity Tracking

All functions designed to meet constitution limits (≤30 lines, ≤5 depth). No violations anticipated.

## Phase 0: Research

### R1. Hardware Requirements Analysis

**Goal**: Identify exact ESP32 pinout, accelerometer I2C configuration, LED/buzzer specifications

**Deliverables**:
- Pin mapping diagram (I2C SDA/SCL, LED GPIO, Buzzer PWM)
- MPU6050 address and register specifications
- WiFi antenna requirements and RSSI expectations

### R2. MQTT Protocol & QoS Selection

**Goal**: Determine optimal QoS level, topic structure, message format

**Deliverables**:
- Topic naming convention (e.g., `airtag/device/{deviceID}/motion`)
- Message payload schema (JSON vs binary)
- QoS recommendation (QoS 0 for speed vs QoS 1 for reliability)

### R3. Accelerometer Motion Detection Algorithm

**Goal**: Research threshold-based vs continuous monitoring, debouncing strategies

**Deliverables**:
- Recommended acceleration threshold (G-force value)
- Debouncing algorithm (time window, sample averaging)
- Interrupt-driven vs polling approach

### R4. Power Consumption & Sleep Modes

**Goal**: Evaluate if sleep modes needed or continuous operation sufficient

**Deliverables**:
- Current consumption estimates (transmitter, receiver)
- Recommendation on light sleep vs deep sleep for battery operation
- (Note: Out of scope for MVP per spec, but good to document)

**Output**: `research.md` with findings and architectural decisions

## Phase 1: Architecture Design

### A1. Hardware Abstraction Layer Design

**Goal**: Define driver interfaces that enable mocking for unit tests

**Contracts**:
```cpp
// include/drivers/MPU6050Driver.h
class IMPU6050Driver {
public:
    virtual bool initialize() = 0;
    virtual bool readAcceleration(float& x, float& y, float& z) = 0;
    virtual ~IMPU6050Driver() = default;
};

// include/drivers/LEDDriver.h  
class ILEDDriver {
public:
    virtual void setState(bool on) = 0;
    virtual void setBrightness(uint8_t level) = 0;
    virtual ~ILEDDriver() = default;
};

// include/drivers/BuzzerDriver.h
class IBuzzerDriver {
public:
    virtual void playTone(uint16_t frequency, uint16_t duration) = 0;
    virtual void stop() = 0;
    virtual ~IBuzzerDriver() = default;
};
```

### A2. Service Layer Interfaces

**Contracts**:
```cpp
// include/services/AccelerometerService.h
class AccelerometerService {
public:
    AccelerometerService(IMPU6050Driver* driver, float threshold);
    bool detectMotion();  // Returns true if motion exceeds threshold
    MovementEvent getLastEvent();
private:
    IMPU6050Driver* mDriver;
    float mThreshold;
    // ... (≤30 lines per function)
};

// include/services/MQTTService.h  
class MQTTService {
public:
    MQTTService(const char* broker, uint16_t port, const char* clientID);
    bool connect();
    bool publish(const char* topic, const MovementEvent& event);
    bool subscribe(const char* topic, void (*callback)(MovementEvent));
    void loop();  // Process MQTT events
private:
    // ... (≤30 lines per function)
};

// include/services/AlertService.h
class AlertService {
public:
    AlertService(ILEDDriver* led, IBuzzerDriver* buzzer);
    void triggerAlert(uint32_t duration);
    void update();  // Call in loop to handle alert timing
private:
    // ... (≤30 lines per function)
};
```

### A3. Data Model Definitions

**Key Entities**:
```cpp
// include/models/MovementEvent.h
struct MovementEvent {
    char deviceID[32];
    unsigned long timestamp;     // millis() since boot
    float accelerationMagnitude; // Combined X/Y/Z magnitude
    enum EventType { MOTION_START, MOTION_CONTINUE, MOTION_STOP } type;
};

// include/models/DeviceConfig.h
struct DeviceConfig {
    char wifiSSID[64];
    char wifiPassword[64];
    char mqttBrokerIP[16];
    uint16_t mqttPort;
    char deviceID[32];
    float sensitivityThreshold;  // G-force (e.g., 0.5)
    uint32_t alertDuration;      // milliseconds
};
```

### A4. State Machine Design

**Transmitter States**: BOOT → WIFI_CONNECTING → MQTT_CONNECTING → MONITORING → PUBLISHING → MONITORING  
**Receiver States**: BOOT → WIFI_CONNECTING → MQTT_CONNECTING → LISTENING → ALERTING → LISTENING

**Output**: `data-model.md`, `contracts/` directory with interface definitions, state machine diagrams

## Phase 2: Test-Driven Development

Per constitution, **tests must be written first** and fail before implementation.

### T1. Unit Tests for Accelerometer Service

**Test Cases**:
- Motion detected when acceleration exceeds threshold
- No motion when below threshold  
- Debouncing prevents rapid-fire events
- Event timestamp and magnitude captured correctly

**Mocking**: Mock IMPU6050Driver returns controlled acceleration values

### T2. Unit Tests for MQTT Service

**Test Cases**:
- Connection established with valid broker
- Publish serializes MovementEvent to JSON correctly
- Subscribe callback invoked on message receipt
- Connection retry with exponential backoff

**Mocking**: Mock WiFiClient for network simulation

### T3. Unit Tests for Alert Service

**Test Cases**:
- Alert activates LED and buzzer on trigger
- Alert deactivates after duration expires
- Multiple triggers extend duration correctly

**Mocking**: Mock ILEDDriver and IBuzzerDriver to verify calls

### T4. Integration Tests

**Test Cases**:
- End-to-end transmitter flow (mock sensor → real MQTT publish to test broker)
- End-to-end receiver flow (test MQTT message → real alert activation)

**Output**: Test suite in `test/` with all tests failing (Red phase)

## Phase 3: Implementation (Green Phase)

Implement code to pass tests, following constitution limits:

### I1. Driver Implementations
- MPU6050Driver.cpp (I2C communication)
- LEDDriver.cpp (GPIO control)
- BuzzerDriver.cpp (PWM tone generation)

### I2. Service Implementations
- AccelerometerService.cpp (motion detection logic)
- MQTTService.cpp (PubSubClient wrapper)
- WiFiService.cpp (connection management)
- AlertService.cpp (timing control)

### I3. Utility Implementations
- Logger.cpp (serial output)
- ConnectionRetry.cpp (exponential backoff)

### I4. Main Programs
- main_transmitter.cpp (initialize, loop with motion detection)
- main_receiver.cpp (initialize, loop with MQTT listening)

**Constitution Compliance**:
- Every function ≤30 lines
- Every function ≤5 nesting depth
- .h declares, .cpp implements
- Classes for stateful (services), structs for data (models)

**Output**: All tests passing (Green phase)

## Phase 4: Refactoring & Documentation

### F1. Code Review for Constitution Compliance
- Verify all functions meet line/depth limits
- Check RAII usage (destructors clean up resources)
- Verify const-correctness

### F2. Performance Optimization
- Reduce heap allocations in hot paths
- Optimize I2C read frequency

### F3. Documentation
- Function-level comments for public APIs
- README with build/flash instructions
- Raspberry Pi broker setup guide

## Phase 5: Hardware Integration & Testing

### H1. Transmitter Hardware Testing
- Flash firmware to ESP32
- Verify accelerometer readings via serial
- Test MQTT publishing to Mosquitto broker
- Validate motion detection sensitivity

### H2. Receiver Hardware Testing  
- Flash firmware to second ESP32
- Verify MQTT subscription
- Test LED and buzzer activation
- Measure alert timing accuracy

### H3. System Integration Testing
- Test full transmitter → broker → receiver flow
- Validate WiFi reconnection after dropout
- Test MQTT reconnection after broker restart
- Measure end-to-end latency

## Raspberry Pi Setup

**Prerequisites**: Raspberry Pi 4 with Raspberry Pi OS, connected to same WiFi network

**Installation Steps**:
```bash
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Configure Mosquitto (allow anonymous for MVP)
echo "listener 1883" | sudo tee -a /etc/mosquitto/mosquitto.conf
echo "allow_anonymous true" | sudo tee -a /etc/mosquitto/mosquitto.conf
sudo systemctl restart mosquitto

# Test broker
mosquitto_sub -h localhost -t test/topic &
mosquitto_pub -h localhost -t test/topic -m "Hello MQTT"
```

**Output**: Documented in `docs/raspberry-pi-setup.md`

## Build & Deployment

### PlatformIO Configuration

Update `platformio.ini`:
```ini
[env:transmitter]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    knolleary/PubSubClient@^2.8
    adafruit/Adafruit MPU6050@^2.2.4
build_src_filter = +<*> -<main_receiver.cpp>

[env:receiver]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    knolleary/PubSubClient@^2.8
build_src_filter = +<*> -<main_transmitter.cpp>
```

### Build Commands
```bash
# Build transmitter
pio run -e transmitter

# Upload transmitter  
pio run -e transmitter -t upload

# Build receiver
pio run -e receiver

# Upload receiver
pio run -e receiver -t upload

# Run tests
pio test
```

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| WiFi connection instability | High | Implement robust retry logic, connection status indicators |
| Accelerometer false positives | Medium | Tunable threshold, debouncing algorithm |
| MQTT message loss | Medium | Use QoS 1 for reliability vs QoS 0 for speed (testing needed) |
| Memory constraints (520KB RAM) | Low | Avoid dynamic allocation, use stack, const strings in flash |
| I2C bus timing issues | Low | Use standard Wire library, proper pull-up resistors |

## Success Metrics Validation

Mapping to spec success criteria:

- **SC-001** (500ms latency): Measure with timestamps in MQTT messages
- **SC-002** (200ms alert): Measure MQTT receive to LED on time  
- **SC-003** (24hr stability): Long-duration soak test with monitoring
- **SC-004** (false negative <1%): Controlled motion test suite (100 samples)
- **SC-005** (30s reconnect): Network outage simulation testing
- **SC-006** (WiFi RSSI -75dBm): Range testing at various distances
- **SC-007** (perceivable alerts): Human perception testing (3m audio, 5m visual)

## Next Steps

1. ✅ Specification validated and ready
2. → Create detailed task breakdown (`/speckit.tasks`)
3. → Begin Phase 0 research
4. → Write tests (Red phase)
5. → Implement features (Green phase)
6. → Refactor and optimize
7. → Hardware integration and validation
