local logger = {}

function logger.set_level(msg)
  return flatt_logger_set_level(msg)
end

function logger.get_level(msg)
  return flatt_logger_get_level(msg)
end

function logger.trace(msg)
  return flatt_logger_trace(msg)
end

function logger.debug(msg)
  return flatt_logger_debug(msg)
end

function logger.info(msg)
  return flatt_logger_info(msg)
end

function logger.warn(msg)
  return flatt_logger_warn(msg)
end

function logger.error(msg)
  return flatt_logger_error(msg)
end

function logger.critical(msg)
  return flatt_logger_critical(msg)
end

function logger.fatal(msg)
  return flatt_logger_fatal(msg)
end

return logger
