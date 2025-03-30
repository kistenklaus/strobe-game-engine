#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "spdlog/common.h"

namespace strobe::logging {

class Logger {
 private:
 public:
  static Logger create();

  void destroy();

 private:
};

#define STROBE_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define STROBE_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define STROBE_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define STROBE_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define STROBE_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

// ─────────────────────────────────────────────────────────────
// Renderer logger — logs to renderer.log and warns/errors to console
// ─────────────────────────────────────────────────────────────
#define STROBE_RENDERER_TRACE(...) \
  SPDLOG_LOGGER_TRACE(spdlog::get("renderer"), __VA_ARGS__)
#define STROBE_RENDERER_DEBUG(...) \
  SPDLOG_LOGGER_DEBUG(spdlog::get("renderer"), __VA_ARGS__)
#define STROBE_RENDERER_INFO(...) \
  SPDLOG_LOGGER_INFO(spdlog::get("renderer"), __VA_ARGS__)
#define STROBE_RENDERER_WARN(...) \
  SPDLOG_LOGGER_WARN(spdlog::get("renderer"), __VA_ARGS__)
#define STROBE_RENDERER_ERROR(...) \
  SPDLOG_LOGGER_ERROR(spdlog::get("renderer"), __VA_ARGS__)

// ─────────────────────────────────────────────────────────────
// Resources logger — logs to resources.log and warns/errors to console
// ─────────────────────────────────────────────────────────────
#define STROBE_RESOURCES_TRACE(...) \
  SPDLOG_LOGGER_TRACE(spdlog::get("resources"), __VA_ARGS__)
#define STROBE_RESOURCES_DEBUG(...) \
  SPDLOG_LOGGER_DEBUG(spdlog::get("resources"), __VA_ARGS__)
#define STROBE_RESOURCES_INFO(...) \
  SPDLOG_LOGGER_INFO(spdlog::get("resources"), __VA_ARGS__)
#define STROBE_RESOURCES_WARN(...) \
  SPDLOG_LOGGER_WARN(spdlog::get("resources"), __VA_ARGS__)
#define STROBE_RESOURCES_ERROR(...) \
  SPDLOG_LOGGER_ERROR(spdlog::get("resources"), __VA_ARGS__)

}  // namespace strobe::logging
   //
