const express = require('express');
const router = express.Router();
const { writeMetric, queryMetrics } = require('../db/influx');
const { validate, hardwareMetricSchema, querySchema } = require('../validation/schemas');
const logger = require('../utils/logger');

// POST /api/hardware - Store hardware metrics
router.post('/', async (req, res, next) => {
  try {
    const { error, value } = validate(req.body, hardwareMetricSchema);
    if (error) {
      error.status = 400;
      return next(error);
    }

    const { deviceId, timestamp, ramUsage, cpuFreqMHz, uptime, freeHeap } = value;

    await writeMetric(deviceId, timestamp, {
      ramUsage,
      cpuFreqMHz,
      uptime,
      freeHeap
    });

    logger.debug('Hardware metric stored', { deviceId, ram: ramUsage });

    res.status(201).json({
      id: `metric_${Date.now()}`,
      stored: true
    });
  } catch (error) {
    logger.error('Failed to store metric:', error.message);
    next(error);
  }
});

// GET /api/hardware/:deviceId - Query hardware metrics
router.get('/:deviceId', async (req, res, next) => {
  try {
    const { deviceId } = req.params;
    const { error, value } = validate(req.query, querySchema);
    
    if (error) {
      error.status = 400;
      return next(error);
    }

    let { from, to } = value;

    // Default to last 24 hours if not specified
    if (!to) to = Date.now();
    if (!from) from = to - (24 * 60 * 60 * 1000); // 24 hours ago

    const metrics = await queryMetrics(deviceId, from, to);

    const formattedMetrics = metrics.map(m => ({
      timestamp: new Date(m.time).toISOString(),
      ramUsage: m.ram_usage,
      cpuFreqMHz: m.cpu_freq_mhz,
      uptime: m.uptime,
      freeHeap: m.free_heap
    }));

    logger.debug('Metrics queried', { deviceId, count: metrics.length });

    res.json({
      deviceId,
      from: new Date(from).toISOString(),
      to: new Date(to).toISOString(),
      metrics: formattedMetrics
    });
  } catch (error) {
    logger.error('Failed to query metrics:', error.message);
    next(error);
  }
});

module.exports = router;
