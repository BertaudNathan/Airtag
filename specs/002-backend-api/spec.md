# Feature Specification: Backend API Server for IoT Data

**Feature ID:** 002-backend-api  
**Status:** Specification  
**Created:** February 11, 2026

---

## Overview

Create a Node.js backend server that exposes REST APIs for storing and retrieving sensor data from IoT devices (ESP32 transmitter and receiver). The server will handle motion events, hardware metrics, and provide time-series data storage for monitoring device performance.

---

## User Scenarios & Testing

### Scenario 1: Store Motion Event from Transmitter
**Actor:** Transmitter ESP32  
**Flow:**
1. ESP32 detects motion via accelerometer
2. Publishes motion event to MQTT topic
3. Raspberry Pi subscribes and collects event
4. Raspberry Pi sends POST request to `/api/events` with motion data
5. Server stores event with timestamp
6. Server responds with created event ID

**Acceptance Criteria:**
- ✅ Request returns 201 Created status
- ✅ Event stored with device ID, timestamp, and acceleration magnitude
- ✅ Response includes unique event ID
- ✅ Duplicate events within same second are handled gracefully

### Scenario 2: Store Hardware Metrics (Time-Series)
**Actor:** Any ESP32 (transmitter or receiver)  
**Flow:**
1. Device periodically sends hardware metrics (RAM, CPU, uptime, heap)
2. Raspberry Pi forwards POST request to `/api/hardware` endpoint
3. Server stores metrics in time-series database
4. Device can retrieve historical data via GET request
5. Time-series data is queryable by time range

**Acceptance Criteria:**
- ✅ Metrics stored with nanosecond precision timestamps
- ✅ Can query metrics for specific device over time range
- ✅ Response includes data points with timestamps
- ✅ Old data automatically pruned (retention policy)

### Scenario 3: Query Hardware Metrics
**Actor:** Web Dashboard / Monitoring System  
**Flow:**
1. Client requests hardware metrics for device over last 24 hours
2. GET `/api/hardware/:deviceId?from=timestamp&to=timestamp`
3. Server retrieves time-series data from database
4. Returns data sorted by timestamp

**Acceptance Criteria:**
- ✅ Returns array of metrics within time range
- ✅ Supports flexible time range queries
- ✅ Response includes timestamp, RAM %, CPU MHz, uptime, free heap
- ✅ Defaults to last 24 hours if no time specified

### Scenario 4: Retrieve Motion Events
**Actor:** Web Dashboard / Alert System  
**Flow:**
1. Client queries motion events for specific device
2. GET `/api/events/:deviceId?limit=50&offset=0`
3. Server returns paginated motion events
4. Includes acceleration magnitude for each event

**Acceptance Criteria:**
- ✅ Supports pagination (limit, offset)
- ✅ Returns events sorted by timestamp (newest first)
- ✅ Includes acceleration magnitude for each event

---

## Functional Requirements

### API Endpoints

#### 1. Store Motion Event
```
POST /api/events
Body: {
  "deviceId": "airtag_transmitter_01",
  "timestamp": 1707625800000,
  "accelerationMagnitude": 3.5,
  "type": "MOTION_START"
}
Response 201: {
  "id": "event_123abc",
  "created": "2026-02-11T10:30:00Z"
}
```

**Requirements:**
- Receive motion events from IoT devices via HTTP POST
- Accept deviceId, timestamp, acceleration magnitude, event type
- Store with server-side timestamp if not provided
- Return unique event ID
- Validate input: deviceId required, magnitude must be positive number

#### 2. Store Hardware Metrics
```
POST /api/hardware
Body: {
  "deviceId": "airtag_receiver_01",
  "timestamp": 1707625800000,
  "ramUsage": "65",
  "cpuFreqMHz": "240",
  "uptime": "3600",
  "freeHeap": "102400"
}
Response 201: {
  "id": "metric_xyz789",
  "stored": true
}
```

**Requirements:**
- Accept hardware metrics with nanosecond-precision timestamps
- Store in time-series database (InfluxDB or equivalent)
- Values stored as strings converted to appropriate types
- Support bulk inserts for efficiency
- Implement data retention policy (keep last 30 days)

#### 3. Query Hardware Metrics
```
GET /api/hardware/:deviceId?from=1707625800000&to=1707712200000
Response 200: {
  "deviceId": "airtag_receiver_01",
  "metrics": [
    {
      "timestamp": "2026-02-11T10:30:00Z",
      "ramUsage": 65,
      "cpuFreqMHz": 240,
      "uptime": 3600,
      "freeHeap": 102400
    }
  ]
}
```

**Requirements:**
- Query time-series data by device ID and time range
- Time range parameters: from, to (milliseconds since epoch)
- Default time range: last 24 hours if not specified
- Return data as JSON array sorted by timestamp ascending
- Support downsampling for large queries [NEEDS CLARIFICATION: granularity preference]

#### 4. Retrieve Motion Events
```
GET /api/events/:deviceId?limit=50&offset=0
Response 200: {
  "total": 234,
  "events": [
    {
      "id": "event_123abc",
      "deviceId": "airtag_transmitter_01",
      "timestamp": "2026-02-11T10:30:00Z",
      "accelerationMagnitude": 3.5,
      "type": "MOTION_START"
    }
  ]
}
```

**Requirements:**
- Retrieve motion events with pagination
- Support limit (default 50, max 500) and offset parameters
- Return total count of events matching query
- Sort by timestamp descending (newest first)
- Include event type for filtering

#### 5. Health Check
```
GET /api/health
Response 200: {
  "status": "healthy",
  "database": "connected",
  "uptime": 12345
}
```

**Requirements:**
- Simple endpoint to verify server is running
- Check database connectivity
- Return uptime in seconds

### Database Schema

#### Events Table
- id (UUID, primary key)
- deviceId (string, indexed)
- timestamp (datetime, indexed)
- accelerationMagnitude (float)
- type (enum: MOTION_START, MOTION_CONTINUE, MOTION_STOP)
- created (datetime, server timestamp)

#### Hardware Metrics (Time-Series)
- timestamp (nanoseconds, indexed)
- deviceId (string, indexed)
- ramUsage (integer, 0-100%)
- cpuFreqMHz (integer)
- uptime (integer, seconds)
- freeHeap (integer, bytes)
- Retention: 30 days
- Aggregation: 1-minute buckets for queries > 24 hours

### Non-Functional Requirements

**Performance:**
- Response time < 200ms for single event POST
- Response time < 500ms for metric queries over 24 hours
- Support concurrent requests from multiple devices

**Reliability:**
- Graceful handling of network failures
- Automatic database reconnection
- Data persistence across server restarts

**Scalability:**
- Support 10+ concurrent IoT devices
- Handle 1000+ events per minute
- Time-series database optimized for write-heavy workload

**Security:**
- [NEEDS CLARIFICATION: Authentication requirement for API endpoints]
- Validate all input parameters
- SQL injection prevention (use parameterized queries)
- Rate limiting on endpoints [NEEDS CLARIFICATION: suggested limits]

---

## Success Criteria

1. **API Functionality**: All 5 endpoints implemented and functional
2. **Data Storage**: Motion events and hardware metrics persist across server restarts
3. **Performance**: All endpoints respond within SLA (< 200ms POST, < 500ms GET)
4. **Time-Series**: Hardware data queryable with configurable retention policy
5. **Reliability**: Server automatically reconnects to database on connection loss
6. **Documentation**: API endpoints documented with examples
7. **Testing**: All endpoints tested with sample requests
8. **Deployment Ready**: Server runs in production-ready configuration

---

## Key Entities

- **Device**: Identified by unique deviceId (airtag_transmitter_01, airtag_receiver_01, etc.)
- **Motion Event**: Acceleration event with timestamp and magnitude
- **Hardware Metric**: System metric (RAM, CPU, heap) with nanosecond timestamp
- **Time Window**: Configurable time range for queries (milliseconds since epoch)

---

## Assumptions

- Timestamps from ESP32 devices are in milliseconds since boot (converted by Raspberry Pi)
- Hardware metrics are sent every 8 loop iterations (per current implementation)
- Motion events are sent immediately after detection
- Server runs on Raspberry Pi or accessible via local network (192.168.54.45:5000)
- PostgreSQL available for event storage (relational)
- InfluxDB or similar time-series DB for hardware metrics (better suited for time-series)
- Node.js v18+ available on deployment environment

---

## Dependencies & Constraints

- **Dependencies**: Node.js, Express.js, PostgreSQL, InfluxDB (or DuckDB for embedded time-series)
- **Network**: Assumes local network, Raspberry Pi can reach backend server
- **Database**: Requires two databases (relational for events, time-series for metrics)
- **Constraints**: Single-server deployment, no distributed database complexity

---

## Open Questions

[NEEDS CLARIFICATION: Should API require authentication (API key, JWT)?]
[NEEDS CLARIFICATION: What rate limiting should be applied per device?]
[NEEDS CLARIFICATION: Should downsampling be automatic for large time ranges?]
