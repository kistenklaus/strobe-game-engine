#include "./Logger.hpp"
#include "spdlog/spdlog.h"

strobe::logging::Logger strobe::logging::Logger::create() {
  using namespace spdlog;
  // Console sink
  auto consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  consoleSink->set_level(level::debug);
  consoleSink->set_pattern("[%9n] %^[%l]%$ (%s:%#): %v");
  auto warningSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  warningSink->set_level(level::warn);
  warningSink->set_pattern("[%9n] %^[%l]%$ (%s:%#): %v");

  auto strobeSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      "logs/strobe.log", true);
  strobeSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  auto strobeLogger = std::make_shared<spdlog::logger>(
      "strobe", spdlog::sinks_init_list{consoleSink, strobeSink});
  strobeLogger->set_level(level::debug);
  register_logger(strobeLogger);

  auto rendererSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      "logs/renderer.log", true);
  rendererSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  auto rendererLogger = std::make_shared<spdlog::logger>(
      "renderer", spdlog::sinks_init_list{rendererSink, warningSink});
  rendererLogger->flush_on(level::info);
  register_logger(rendererLogger);

  auto resourcesSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      "logs/resources.log", true);
  resourcesSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  auto resourceLogger = std::make_shared<spdlog::logger>(
      "resources", spdlog::sinks_init_list{resourcesSink, warningSink});
  resourceLogger->flush_on(level::info);
  register_logger(resourceLogger);

  spdlog::set_default_logger(strobeLogger);

  return {};
}

void strobe::logging::Logger::destroy() {
  spdlog::shutdown();
}

