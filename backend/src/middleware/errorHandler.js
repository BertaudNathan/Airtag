const logger = require('../utils/logger');

function errorHandler(err, req, res, next) {
  logger.error('Request error:', {
    message: err.message,
    stack: err.stack,
    path: req.path,
    method: req.method
  });

  // Validation errors
  if (err.isJoi) {
    return res.status(400).json({
      error: 'Validation error',
      details: err.details.map(d => ({
        field: d.path.join('.'),
        message: d.message
      }))
    });
  }

  // Database errors
  if (err.code === 'ECONNREFUSED') {
    return res.status(503).json({
      error: 'Database connection failed'
    });
  }

  // Default error
  res.status(err.status || 500).json({
    error: err.message || 'Internal server error'
  });
}

module.exports = { errorHandler };
