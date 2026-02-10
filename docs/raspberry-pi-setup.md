# MQTT AirTag System - Raspberry Pi Broker Setup

This guide explains how to set up a Raspberry Pi as an MQTT broker for the AirTag tracking system.

## Prerequisites

- Raspberry Pi 4 (or Pi 3 B+) with Raspberry Pi OS installed
- Network connection (WiFi or Ethernet)
- SSH access or direct keyboard/monitor connection
- Same WiFi network as ESP32 devices

## Installation Steps

### 1. Update System

```bash
sudo apt update
sudo apt upgrade -y
```

### 2. Install Mosquitto MQTT Broker

```bash
sudo apt install -y mosquitto mosquitto-clients
```

### 3. Enable Mosquitto Service

```bash
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

### 4. Configure Mosquitto for Network Access

By default, Mosquitto only listens on localhost. Configure it for network access:

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Add these lines at the end of the file:

```
listener 1883
allow_anonymous true
```

**Security Note**: This configuration allows anonymous connections. For production use, configure authentication.

### 5. Restart Mosquitto

```bash
sudo systemctl restart mosquitto
```

### 6. Check Status

```bash
sudo systemctl status mosquitto
```

You should see "active (running)" in green.

### 7. Get Raspberry Pi IP Address

```bash
hostname -I
```

Note the first IP address (e.g., `192.168.1.100`). You'll need this for the ESP32 configuration.

## Testing the Broker

### Test Local Connection

Open two terminal windows on the Raspberry Pi:

**Terminal 1 - Subscribe:**
```bash
mosquitto_sub -h localhost -t test/topic
```

**Terminal 2 - Publish:**
```bash
mosquitto_pub -h localhost -t test/topic -m "Hello MQTT!"
```

You should see "Hello MQTT!" appear in Terminal 1.

### Test Network Connection

From another computer on the same network:

```bash
mosquitto_sub -h <RASPBERRY_PI_IP> -t test/topic
```

## Configure ESP32 Devices

Update the configuration in both `main_transmitter.cpp` and `main_receiver.cpp`:

```cpp
strcpy(config.wifiSSID, "YOUR_WIFI_NETWORK_NAME");
strcpy(config.wifiPassword, "YOUR_WIFI_PASSWORD");
strcpy(config.mqttBrokerIP, "192.168.1.100");  // Your Raspberry Pi IP
```

## Monitoring MQTT Traffic

To monitor all messages on the broker:

```bash
mosquitto_sub -h localhost -t '#' -v
```

This subscribes to all topics (`#` is a wildcard).

To monitor only AirTag motion events:

```bash
mosquitto_sub -h localhost -t 'airtag/motion' -v
```

## Troubleshooting

### Firewall Issues

If ESP32 devices can't connect, check firewall:

```bash
sudo ufw allow 1883/tcp
```

### Check Mosquitto Logs

```bash
sudo tail -f /var/log/mosquitto/mosquitto.log
```

### Restart Mosquitto

```bash
sudo systemctl restart mosquitto
```

### Check Port Binding

```bash
sudo netstat -tulpn | grep 1883
```

You should see Mosquitto listening on port 1883.

## Optional: Enable Authentication

For production deployments, enable authentication:

### 1. Create Password File

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd airtag_user
```

Enter password when prompted.

### 2. Update Configuration

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Change:
```
allow_anonymous false
password_file /etc/mosquitto/passwd
```

### 3. Restart Mosquitto

```bash
sudo systemctl restart mosquitto
```

### 4. Update ESP32 Code

You'll need to modify the MQTT client code to include username/password authentication.

## Network Configuration

### Static IP (Recommended)

Set a static IP for the Raspberry Pi to avoid IP address changes:

```bash
sudo nano /etc/dhcpcd.conf
```

Add (adjust for your network):
```
interface wlan0
static ip_address=192.168.1.100/24
static routers=192.168.1.1
static domain_name_servers=192.168.1.1 8.8.8.8
```

Reboot:
```bash
sudo reboot
```

## Performance Tuning

For higher message rates (optional):

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Add:
```
max_inflight_messages 100
max_queued_messages 1000
```

## System Resources

Check Mosquitto resource usage:

```bash
ps aux | grep mosquitto
```

Mosquitto is lightweight - typically uses <10MB RAM.

## Auto-Start on Boot

Mosquitto is configured to start automatically. To verify:

```bash
sudo systemctl is-enabled mosquitto
```

Should output: `enabled`

## Additional Tools

### MQTT Explorer (Desktop GUI)

Download MQTT Explorer for visual monitoring:
- Website: http://mqtt-explorer.com/
- Connect to: `mqtt://<RASPBERRY_PI_IP>:1883`

### Mobile Apps

- **IoT MQTT Panel** (Android)
- **MQTTool** (iOS)

Both can subscribe to `airtag/motion` for mobile monitoring.

## Summary

Your Raspberry Pi is now running an MQTT broker that:
- Listens on port 1883
- Accepts connections from ESP32 devices on the same network
- Routes motion detection messages from transmitter to receiver
- Provides monitoring and debugging capabilities

Update your ESP32 configuration with the Raspberry Pi IP address and power on the devices!
