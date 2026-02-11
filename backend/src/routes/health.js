const express = require('express');
const router = express.Router();
const { query } = require('../db/postgres');
const logger = require('../utils/logger');

const startTime = Date.now();

// GET /api/health - Health check endpoint
router.get('/', async (req, res) => {
  try {
    // Check database connection
    const result = await query('SELECT NOW()');
    const dbConnected = result.rows.length > 0;

    const uptime = Math.floor((Date.now() - startTime) / 1000);

    res.json({
      status: dbConnected ? 'healthy' : 'degraded',
      database: dbConnected ? 'connected' : 'disconnected',
      uptime,
      timestamp: new Date().toISOString()
    });
  } catch (error) {
    logger.warn('Health check failed:', error.message);
    
    const uptime = Math.floor((Date.now() - startTime) / 1000);
    
    res.status(503).json({
      status: 'unhealthy',
      database: 'disconnected',
      uptime,
      timestamp: new Date().toISOString()
    });
  }
});

module.exports = router;
