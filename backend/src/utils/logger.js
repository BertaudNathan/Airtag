const LOG_LEVEL = process.env.LOG_LEVEL || 'info';

const levels = {
  debug: 0,
  info: 1,
  warn: 2,
  error: 3
};

const colors = {
  reset: '\x1b[0m',
  debug: '\x1b[36m',
  info: '\x1b[32m',
  warn: '\x1b[33m',
  error: '\x1b[31m'
};

function log(level, message, data) {
  if (levels[level] < levels[LOG_LEVEL]) return;
  
  const timestamp = new Date().toISOString();
  const color = colors[level] || '';
  const reset = colors.reset;
  
  let output = `${color}[${timestamp}] ${level.toUpperCase()}:${reset} ${message}`;
  
  if (data) {
    output += ` ${JSON.stringify(data)}`;
  }
  
  console.log(output);
}

module.exports = {
  debug: (msg, data) => log('debug', msg, data),
  info: (msg, data) => log('info', msg, data),
  warn: (msg, data) => log('warn', msg, data),
  error: (msg, data) => log('error', msg, data)
};
