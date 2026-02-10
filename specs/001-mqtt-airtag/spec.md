# Feature Specification: MQTT-Based AirTag Tracking System

**Feature Branch**: `001-mqtt-airtag`  
**Created**: 2026-02-10  
**Status**: Draft  
**Input**: User description: "ESP32 MQTT-based AirTag tracking system with accelerometer detection and alert notification"
**Constitution**: See `.specify/memory/constitution.md` for TDD, architecture, and quality requirements

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Motion Detection and Publishing (Priority: P1)

A tracked object (e.g., backpack, luggage) equipped with an ESP32 transmitter detects movement via an accelerometer and publishes an alert to the MQTT broker, enabling remote monitoring of the object's movement status.

**Why this priority**: This is the core sensing capability - without motion detection and publishing, no alerts can be generated. This forms the foundation of the entire system.

**Independent Test**: Can be fully tested by moving the transmitter ESP32 device and verifying that MQTT messages are published to the broker (observable via MQTT client or broker logs). Delivers immediate value as a standalone motion detector.

**Acceptance Scenarios**:

1. **Given** the transmitter ESP32 is powered on and connected to WiFi and MQTT broker, **When** the device experiences movement exceeding the sensitivity threshold, **Then** an MQTT message containing movement event data is published to the configured topic within 500ms
2. **Given** the transmitter is stationary, **When** no movement is detected for the configured time window, **Then** no MQTT messages are published
3. **Given** the transmitter loses WiFi or MQTT connection, **When** movement is detected, **Then** the device attempts to reconnect and buffer events (or indicate connection failure via LED)

---

### User Story 2 - Alert Reception and Notification (Priority: P2)

A receiver ESP32 device subscribes to movement alerts and provides audio-visual feedback (LED and buzzer) when a tracked object is moved, alerting the user to potential theft or unauthorized access.

**Why this priority**: This completes the user-facing alert system and provides immediate actionable feedback. Without this, motion detection has no user-visible output.

**Independent Test**: Can be fully tested by publishing test MQTT messages to the configured topic and verifying that the receiver activates LEDs and buzzer. Delivers value as a standalone alert notification system (could receive alerts from any MQTT publisher).

**Acceptance Scenarios**:

1. **Given** the receiver ESP32 is powered on and subscribed to the movement topic, **When** a movement message is received, **Then** the LED activates (e.g., flashing red) and buzzer sounds for a configurable duration (default 5 seconds)
2. **Given** the receiver is actively alerting, **When** the alert duration expires, **Then** the LED and buzzer deactivate automatically
3. **Given** multiple movement messages are received rapidly, **When** alerts overlap, **Then** the alert duration extends (does not restart) to avoid excessive notification fatigue

---

### User Story 3 - WiFi Network Configuration (Priority: P3)

Both ESP32 devices connect to a common WiFi network on startup, enabling communication with the Raspberry Pi MQTT broker.

**Why this priority**: Essential infrastructure but lower priority because it's a one-time setup task. Can be hardcoded initially and enhanced later with WiFi provisioning UI.

**Independent Test**: Can be fully tested by powering on each ESP32 and verifying successful WiFi connection (via serial monitor or LED indicator). Delivers value as standalone WiFi client functionality.

**Acceptance Scenarios**:

1. **Given** the ESP32 device is powered on with valid WiFi credentials configured, **When** the device boots up, **Then** it connects to the WiFi network within 10 seconds and indicates success via LED (e.g., solid blue)
2. **Given** the WiFi network is unavailable, **When** the device attempts to connect, **Then** it retries connection with exponential backoff up to 5 attempts, then enters error state indicated by LED (e.g., blinking red)
3. **Given** the device loses WiFi connection during operation, **When** connection is lost, **Then** the device automatically attempts to reconnect without requiring reboot

---

### User Story 4 - MQTT Broker Setup on Raspberry Pi (Priority: P3)

A Raspberry Pi hosts an MQTT broker (e.g., Mosquitto) that facilitates message routing between the transmitter and receiver ESP32 devices.

**Why this priority**: Required infrastructure but can be set up manually in early testing. Lower priority for development as it uses existing software (Mosquitto installation).

**Independent Test**: Can be fully tested by installing Mosquitto on Raspberry Pi and verifying connectivity using MQTT client tools. Delivers value as a standalone MQTT broker for any IoT project.

**Acceptance Scenarios**:

1. **Given** the Raspberry Pi has Mosquitto installed and running, **When** an ESP32 client attempts to connect, **Then** the connection is accepted and the client can publish/subscribe
2. **Given** the broker is running, **When** a message is published to a topic, **Then** all subscribed clients receive the message within 100ms
3. **Given** the broker loses power and restarts, **When** clients reconnect, **Then** they successfully re-establish subscriptions without manual intervention

---

### Edge Cases

- What happens when the accelerometer detects continuous vibration (e.g., device in moving vehicle)? → System should implement debouncing to avoid message flooding
- How does the system handle WiFi network switching or credential changes? → Requires re-configuration (initial version requires re-flash; future: WiFi provisioning)
- What happens when the MQTT broker is offline? → ESP32 devices should retry connection with exponential backoff and indicate connection status
- How does the system handle multiple transmitters? → Each transmitter publishes to unique topic or includes device ID in payload
- What happens when the receiver loses power during an alert? → Alert stops; no persistent alert storage in MVP
- How sensitive should the accelerometer be? → Configurable threshold (default: detect human handling, ignore minor environmental vibration)

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Transmitter ESP32 MUST read accelerometer data continuously and detect motion events based on configurable acceleration threshold
- **FR-002**: Transmitter ESP32 MUST publish MQTT messages containing device ID, timestamp, and acceleration magnitude when motion is detected
- **FR-003**: Transmitter ESP32 MUST connect to a configured WiFi network and MQTT broker on startup
- **FR-004**: Receiver ESP32 MUST subscribe to the configured MQTT topic and listen for movement alert messages
- **FR-005**: Receiver ESP32 MUST activate LED indicators and buzzer output when movement alert message is received
- **FR-006**: Receiver ESP32 MUST deactivate alert outputs after a configurable timeout period
- **FR-007**: Both ESP32 devices MUST indicate WiFi connection status via LED (connected, disconnecting, error states)
- **FR-008**: Both ESP32 devices MUST indicate MQTT broker connection status via LED or serial output
- **FR-009**: System MUST implement connection retry logic with exponential backoff for WiFi and MQTT failures
- **FR-010**: Raspberry Pi MUST run an MQTT broker (Mosquitto) accessible by ESP32 devices on the local network
- **FR-011**: Transmitter MUST implement motion debouncing to prevent message flooding during continuous movement
- **FR-012**: System MUST support configurable parameters: WiFi SSID/password, MQTT broker IP, sensitivity threshold, alert duration

### Key Entities

- **MovementEvent**: Represents a detected motion event with attributes: deviceID (string), timestamp (epoch ms), accelerationMagnitude (float), eventType (enum: motion_start, motion_continue, motion_stop)
- **DeviceConfiguration**: Contains device settings: wifiSSID, wifiPassword, mqttBrokerIP, mqttPort, deviceID, sensitivityThreshold (G-force), alertDuration (ms)
- **AlertState**: Represents current alert status on receiver: isAlerting (bool), alertStartTime (epoch ms), alertEndTime (epoch ms), lastEventReceived (MovementEvent)
- **ConnectionStatus**: Tracks network connectivity: wifiConnected (bool), wifiRSSI (int), mqttConnected (bool), lastReconnectAttempt (epoch ms), reconnectAttempts (int)

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Transmitter detects and publishes motion events within 500ms of movement detection
- **SC-002**: Receiver activates alert (LED + buzzer) within 200ms of receiving MQTT message
- **SC-003**: System maintains stable MQTT connection for 24+ hours of continuous operation without manual intervention
- **SC-004**: Motion detection has <1% false negative rate (does not miss actual movements) and <5% false positive rate (does not trigger on environmental vibration)
- **SC-005**: Both ESP32 devices successfully reconnect to WiFi and MQTT broker within 30 seconds after temporary network outage
- **SC-006**: System operates reliably with WiFi RSSI down to -75 dBm (moderate signal strength)
- **SC-007**: Alert notification is clearly perceivable (audible buzzer from 3 meters, visible LED from 5 meters in normal indoor lighting)

## Assumptions *(optional)*

- **A-001**: ESP32 devices and Raspberry Pi are all connected to the same local WiFi network (no internet connectivity required)
- **A-002**: Raspberry Pi has static or predictable IP address for MQTT broker configuration
- **A-003**: Initial configuration (WiFi credentials, MQTT broker IP) is hardcoded and requires re-compilation to change
- **A-004**: System operates in a typical indoor environment (temperature 0-40°C, no water immersion)
- **A-005**: Power supply is stable (USB power or battery with sufficient capacity)
- **A-006**: Accelerometer used is MPU6050 or similar I2C-based 3-axis accelerometer
- **A-007**: MQTT communication is unencrypted (no TLS) for MVP; authentication optional

## Out of Scope *(optional)*

- **OS-001**: GPS location tracking - system only detects motion, not location
- **OS-002**: Internet connectivity or cloud integration - system operates entirely on local network
- **OS-003**: Mobile app interface - configuration is done via code, monitoring via MQTT clients
- **OS-004**: Battery level monitoring and low-power sleep modes - devices assumed to have continuous power
- **OS-005**: Multiple receiver support with synchronized alerts - single receiver in MVP
- **OS-006**: Historical event logging and persistence - no database or event storage
- **OS-007**: Wireless firmware updates (OTA) - updates require USB connection
- **OS-008**: Encryption and authentication - MQTT operates in open mode for MVP
