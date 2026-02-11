const InfluxDB = require('influx');
const logger = require('../utils/logger');

let influxClient;

async function initializeInflux() {
  try {
    influxClient = new InfluxDB.InfluxDB({
      host: process.env.INFLUX_HOST || 'localhost',
      port: process.env.INFLUX_PORT || 8086,
      database: process.env.INFLUX_DATABASE || 'airtag_metrics',
      username: process.env.INFLUX_USER || 'airtag',
      password: process.env.INFLUX_PASSWORD || 'airtag_pass'
    });

    // test connection
    const databases = await influxClient.getDatabaseNames();
    if (!databases.includes(process.env.INFLUX_DATABASE || 'airtag_metrics')) {
      await influxClient.createDatabase(process.env.INFLUX_DATABASE || 'airtag_metrics');
      logger.info('InfluxDB database created');
    }

    logger.info('InfluxDB initialized');
  } catch (error) {
    logger.warn('InfluxDB not available, using fallback storage:', error.message);
    // In production, you may want to fail here
    // For development, we'll continue without it
  }
}

async function writeMetric(deviceId, timestamp, metrics) {
  if (!influxClient) {
    logger.warn('InfluxDB client not available');
    return;
  }

  try {
    await influxClient.writePoints([
      {
        measurement: 'hardware_metrics',
        tags: {
          device_id: deviceId
        },
        fields: {
          ram_usage: parseInt(metrics.ramUsage),
          cpu_freq_mhz: parseInt(metrics.cpuFreqMHz),
          uptime: parseInt(metrics.uptime),
          free_heap: parseInt(metrics.freeHeap)
        },
        timestamp: timestamp * 1000000 // Convert ms to nanoseconds
      }
    ], {
      database: process.env.INFLUX_DATABASE || 'airtag_metrics'
    });
  } catch (error) {
    logger.error('Failed to write metric to InfluxDB:', error.message);
  }
}

async function queryMetrics(deviceId, fromMs, toMs) {
  if (!influxClient) {
    logger.warn('InfluxDB client not available');
    return [];
  }

  try {
    const result = await influxClient.query(`
      SELECT * FROM hardware_metrics 
      WHERE device_id = '${deviceId}'
      AND time >= ${fromMs * 1000000}ns 
      AND time <= ${toMs * 1000000}ns
      ORDER BY time ASC
    `, {
      database: process.env.INFLUX_DATABASE || 'airtag_metrics'
    });

    return result[0] || [];
  } catch (error) {
    logger.error('Failed to query metrics:', error.message);
    return [];
  }
}

module.exports = {
  initializeInflux,
  writeMetric,
  queryMetrics
};
