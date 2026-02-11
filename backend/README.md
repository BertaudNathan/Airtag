# AirTag Backend API Server

A Node.js Express server that provides REST APIs for storing and retrieving IoT sensor data from ESP32 devices via Raspberry Pi.

## Features

- ✅ REST API for motion events storage
- ✅ Time-series database for hardware metrics (RAM, CPU, uptime, heap)
- ✅ Pagination support for event queries
- ✅ Time-range queries for hardware metrics
- ✅ Health check endpoint
- ✅ Input validation with Joi
- ✅ Comprehensive error handling
- ✅ PostgreSQL for relational data
- ✅ InfluxDB for time-series data

## Installation

```bash
cd backend
npm install
```

## Configuration

1. Copy `.env.example` to `.env`:
```bash
cp .env.example .env
```

2. Update `.env` with your database credentials:
```
PG_HOST=localhost
PG_PORT=5432
PG_DATABASE=airtag
PG_USER=airtag_user
PG_PASSWORD=your_password

INFLUX_HOST=localhost
INFLUX_PORT=8086
INFLUX_DATABASE=airtag_metrics
```

## Docker Setup (Recommended)

Start PostgreSQL and InfluxDB with Docker Compose:

```bash
docker-compose -f docker-compose.yml up -d
```

### docker-compose.yml:
```yaml
version: '3.8'

services:
  postgres:
    image: postgres:15-alpine
    environment:
      POSTGRES_DB: airtag
      POSTGRES_USER: airtag_user
      POSTGRES_PASSWORD: airtag_pass
    ports:
      - "5432:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data

  influxdb:
    image: influxdb:2.7
    environment:
      INFLUXDB_DB: airtag_metrics
      INFLUXDB_ADMIN_USER: airtag
      INFLUXDB_ADMIN_PASSWORD: airtag_pass
    ports:
      - "8086:8086"
    volumes:
      - influx_data:/var/lib/influxdb

volumes:
  postgres_data:
  influx_data:
```

## Running the Server

### Development:
```bash
npm run dev
```

### Production:
```bash
npm start
```

Server runs on `http://localhost:5000`

## API Endpoints

### 1. Health Check
```
GET /api/health
```

**Response:**
```json
{
  "status": "healthy",
  "database": "connected",
  "uptime": 123,
  "timestamp": "2026-02-11T10:30:00Z"
}
```

### 2. Store Motion Event
```
POST /api/events
Content-Type: application/json

{
  "deviceId": "airtag_transmitter_01",
  "timestamp": 1707625800000,
  "accelerationMagnitude": 3.5,
  "type": "MOTION_START"
}
```

**Response (201):**
```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "created": "2026-02-11T10:30:00Z"
}
```

### 3. Query Motion Events
```
GET /api/events/airtag_transmitter_01?limit=50&offset=0
```

**Response:**
```json
{
  "total": 234,
  "limit": 50,
  "offset": 0,
  "events": [
    {
      "id": "550e8400-e29b-41d4-a716-446655440000",
      "deviceId": "airtag_transmitter_01",
      "timestamp": "2026-02-11T10:30:00Z",
      "accelerationMagnitude": 3.5,
      "type": "MOTION_START"
    }
  ]
}
```

### 4. Store Hardware Metric
```
POST /api/hardware
Content-Type: application/json

{
  "deviceId": "airtag_receiver_01",
  "timestamp": 1707625800000,
  "ramUsage": "65",
  "cpuFreqMHz": "240",
  "uptime": "3600",
  "freeHeap": "102400"
}
```

**Response (201):**
```json
{
  "id": "metric_1707625800000",
  "stored": true
}
```

### 5. Query Hardware Metrics
```
GET /api/hardware/airtag_receiver_01?from=1707625800000&to=1707712200000
```

**Default (last 24 hours):**
```
GET /api/hardware/airtag_receiver_01
```

**Response:**
```json
{
  "deviceId": "airtag_receiver_01",
  "from": "2026-02-10T10:30:00Z",
  "to": "2026-02-11T10:30:00Z",
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

## Environment Variables

```
NODE_ENV              - development or production
PORT                  - Server port (default: 5000)
PG_HOST              - PostgreSQL host
PG_PORT              - PostgreSQL port
PG_DATABASE          - PostgreSQL database name
PG_USER              - PostgreSQL user
PG_PASSWORD          - PostgreSQL password
INFLUX_HOST          - InfluxDB host
INFLUX_PORT          - InfluxDB port
INFLUX_DATABASE      - InfluxDB database name
INFLUX_USER          - InfluxDB user
INFLUX_PASSWORD      - InfluxDB password
LOG_LEVEL            - debug, info, warn, error (default: info)
CORS_ORIGIN          - CORS origin (default: *)
HARDWARE_RETENTION   - Data retention in days (default: 30)
```

## Project Structure

```
backend/
├── src/
│   ├── index.js                 # Entry point
│   ├── db/
│   │   ├── postgres.js          # PostgreSQL client
│   │   └── influx.js            # InfluxDB client
│   ├── routes/
│   │   ├── events.js            # Motion events endpoints
│   │   ├── hardware.js          # Hardware metrics endpoints
│   │   └── health.js            # Health check endpoint
│   ├── validation/
│   │   └── schemas.js           # Input validation schemas
│   ├── middleware/
│   │   └── errorHandler.js      # Global error handler
│   └── utils/
│       └── logger.js            # Logger utility
├── package.json
├── .env.example
└── README.md
```

## Integration with Raspberry Pi

On your Raspberry Pi, modify the MQTT subscriber to forward data to this API:

```javascript
// Example Node.js MQTT subscriber on Raspberry Pi
const mqtt = require('mqtt');
const axios = require('axios');

const client = mqtt.connect('mqtt://localhost:1883');

client.on('message', async (topic, message) => {
  const data = JSON.parse(message);
  
  try {
    if (topic === 'airtag/motion') {
      await axios.post('http://192.168.54.45:5000/api/events', {
        deviceId: data.deviceID,
        timestamp: data.timestamp,
        accelerationMagnitude: data.accelerationMagnitude,
        type: data.type
      });
    } else if (topic === 'airtag/hardware') {
      await axios.post('http://192.168.54.45:5000/api/hardware', {
        deviceId: data.deviceID,
        timestamp: data.timestamp,
        ramUsage: data.ramUsage,
        cpuFreqMHz: data.cpuFreqMHz,
        uptime: data.uptime,
        freeHeap: data.freeHeap
      });
    }
  } catch (error) {
    console.error('Failed to forward data:', error.message);
  }
});
```

## Testing

```bash
npm test
```

## License

MIT
