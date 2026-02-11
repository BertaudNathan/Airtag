const express = require('express');
const router = express.Router();
const { query } = require('../db/postgres');
const { validate, eventSchema, querySchema } = require('../validation/schemas');
const logger = require('../utils/logger');

// POST /api/events - Store a motion event
router.post('/', async (req, res, next) => {
  try {
    const { error, value } = validate(req.body, eventSchema);
    if (error) {
      error.status = 400;
      return next(error);
    }

    const { deviceId, timestamp, accelerationMagnitude, type } = value;

    const result = await query(
      `INSERT INTO events (device_id, timestamp, acceleration_magnitude, type)
       VALUES ($1, $2, $3, $4)
       RETURNING id, created`,
      [deviceId, timestamp, accelerationMagnitude, type || 'MOTION_START']
    );

    const event = result.rows[0];
    logger.debug('Event stored', { deviceId, eventId: event.id });

    res.status(201).json({
      id: event.id,
      created: event.created
    });
  } catch (error) {
    logger.error('Failed to store event:', error.message);
    next(error);
  }
});

// GET /api/events/:deviceId - Retrieve motion events for a device
router.get('/:deviceId', async (req, res, next) => {
  try {
    const { deviceId } = req.params;
    const { error, value } = validate(req.query, querySchema);
    
    if (error) {
      error.status = 400;
      return next(error);
    }

    const { limit, offset } = value;

    // Get total count
    const countResult = await query(
      'SELECT COUNT(*) as count FROM events WHERE device_id = $1',
      [deviceId]
    );
    const total = parseInt(countResult.rows[0].count);

    // Get paginated events
    const result = await query(
      `SELECT id, device_id, timestamp, acceleration_magnitude, type, created
       FROM events
       WHERE device_id = $1
       ORDER BY timestamp DESC
       LIMIT $2 OFFSET $3`,
      [deviceId, limit, offset]
    );

    const events = result.rows.map(row => ({
      id: row.id,
      deviceId: row.device_id,
      timestamp: new Date(row.created).toISOString(),
      accelerationMagnitude: row.acceleration_magnitude,
      type: row.type
    }));

    logger.debug('Events retrieved', { deviceId, count: events.length, total });

    res.json({
      total,
      limit,
      offset,
      events
    });
  } catch (error) {
    logger.error('Failed to retrieve events:', error.message);
    next(error);
  }
});

module.exports = router;
