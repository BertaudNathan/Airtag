const { Pool } = require('pg');
const logger = require('../utils/logger');

const pool = new Pool({
  host: process.env.PG_HOST || 'localhost',
  port: process.env.PG_PORT || 5432,
  database: process.env.PG_DATABASE || 'airtag',
  user: process.env.PG_USER || 'airtag_user',
  password: process.env.PG_PASSWORD || 'airtag_pass',
  max: 20,
  idleTimeoutMillis: 30000,
  connectionTimeoutMillis: 2000,
});

pool.on('error', (err) => {
  logger.error('Unexpected error on idle client', err);
});

async function initializeDatabase() {
  const client = await pool.connect();
  try {
    // Create events table
    await client.query(`
      CREATE TABLE IF NOT EXISTS events (
        id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
        device_id VARCHAR(255) NOT NULL,
        timestamp BIGINT NOT NULL,
        acceleration_magnitude FLOAT NOT NULL,
        type VARCHAR(50),
        created TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        CONSTRAINT acceleration_positive CHECK (acceleration_magnitude >= 0)
      );
      
      CREATE INDEX IF NOT EXISTS idx_events_device_id ON events(device_id);
      CREATE INDEX IF NOT EXISTS idx_events_timestamp ON events(timestamp DESC);
      CREATE INDEX IF NOT EXISTS idx_events_device_timestamp ON events(device_id, timestamp DESC);
    `);
    
    logger.info('Database tables initialized');
  } catch (error) {
    logger.error('Failed to initialize database:', error.message);
    throw error;
  } finally {
    client.release();
  }
}

async function query(text, params) {
  return pool.query(text, params);
}

async function close() {
  await pool.end();
}

module.exports = {
  query,
  pool,
  initializeDatabase,
  close
};
