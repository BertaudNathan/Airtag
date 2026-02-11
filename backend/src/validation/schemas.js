const Joi = require('joi');

const eventSchema = Joi.object({
  deviceId: Joi.string().required().max(255),
  timestamp: Joi.number().integer().required(),
  accelerationMagnitude: Joi.number().min(0).required(),
  type: Joi.string().valid('MOTION_START', 'MOTION_CONTINUE', 'MOTION_STOP').optional()
});

const hardwareMetricSchema = Joi.object({
  deviceId: Joi.string().required().max(255),
  timestamp: Joi.number().integer().required(),
  ramUsage: Joi.string().required(),
  cpuFreqMHz: Joi.string().required(),
  uptime: Joi.string().required(),
  freeHeap: Joi.string().required()
});

const querySchema = Joi.object({
  from: Joi.number().integer().optional(),
  to: Joi.number().integer().optional(),
  limit: Joi.number().integer().min(1).max(500).default(50).optional(),
  offset: Joi.number().integer().min(0).default(0).optional()
});

function validate(data, schema) {
  const { error, value } = schema.validate(data, {
    abortEarly: false,
    stripUnknown: true
  });

  if (error) {
    error.isJoi = true;
  }

  return { error, value };
}

module.exports = {
  validate,
  eventSchema,
  hardwareMetricSchema,
  querySchema
};
