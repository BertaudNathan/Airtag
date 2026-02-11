require('dotenv').config();
const express = require('express');
const cors = require('cors');
const morgan = require('morgan');
const { initializeDatabase } = require('./db/postgres');
const { initializeInflux } = require('./db/influx');
const eventsRouter = require('./routes/events');
const hardwareRouter = require('./routes/hardware');
const healthRouter = require('./routes/health');
const { errorHandler } = require('./middleware/errorHandler');
const logger = require('./utils/logger');

const app = express();
const PORT = process.env.PORT || 5000;

// Middleware
app.use(cors({
  origin: process.env.CORS_ORIGIN || '*'
}));
app.use(morgan('combined'));
app.use(express.json({ limit: process.env.MAX_REQUEST_SIZE || '1mb' }));
app.use(express.urlencoded({ limit: process.env.MAX_REQUEST_SIZE || '1mb', extended: true }));

// Routes
app.use('/api/health', healthRouter);
app.use('/api/events', eventsRouter);
app.use('/api/hardware', hardwareRouter);

// 404 handler
app.use((req, res) => {
  res.status(404).json({ error: 'Endpoint not found' });
});

// Error handler (must be last)
app.use(errorHandler);

// Initialize databases and start server
async function startServer() {
  try {
    logger.info('Initializing databases...');
    
    // Initialize PostgreSQL
    await initializeDatabase();
    logger.info('PostgreSQL connected');
    
    // Initialize InfluxDB
    await initializeInflux();
    logger.info('InfluxDB connected');
    
    // Start server
    app.listen(PORT, () => {
      logger.info(`Server running on http://localhost:${PORT}`);
      logger.info(`Environment: ${process.env.NODE_ENV || 'development'}`);
    });
  } catch (error) {
    logger.error('Failed to start server:', error);
    process.exit(1);
  }
}

// Handle graceful shutdown
process.on('SIGTERM', () => {
  logger.info('SIGTERM received, shutting down gracefully...');
  process.exit(0);
});

process.on('SIGINT', () => {
  logger.info('SIGINT received, shutting down gracefully...');
  process.exit(0);
});

startServer();

module.exports = app;
