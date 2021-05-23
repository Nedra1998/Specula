#ifndef SPECULA_UTIL_LOG_HPP
#define SPECULA_UTIL_LOG_HPP

#include <memory>

#undef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#define SPDLOG_FMT_EXTERNAL

#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#if defined(_WIN32) || defined(_WIN64)
#include <spdlog/sinks/msvc_sink.h>
#else
#include <spdlog/sinks/syslog_sink.h>
#endif
#include <spdlog/spdlog.h>

#define SPECULA_LOG(logger, level, ...)                                        \
  specula::logging::get(logger)->log(                                          \
      spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level,          \
      __VA_ARGS__)

#define LTRACE(logger, ...)                                                 \
  SPECULA_LOG(logger, spdlog::level::trace, __VA_ARGS__)
#define LDEBUG(logger, ...)                                                 \
  SPECULA_LOG(logger, spdlog::level::debug, __VA_ARGS__)
#define LINFO(logger, ...)                                                  \
  SPECULA_LOG(logger, spdlog::level::info, __VA_ARGS__)
#define LWARN(logger, ...)                                                  \
  SPECULA_LOG(logger, spdlog::level::warn, __VA_ARGS__)
#define LERR(logger, ...)                                                   \
  SPECULA_LOG(logger, spdlog::level::err, __VA_ARGS__)
#define LCRITICAL(logger, ...)                                              \
  SPECULA_LOG(logger, spdlog::level::critical, __VA_ARGS__)

namespace specula::logging {
extern std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink;
void configure_logging();

inline std::shared_ptr<spdlog::logger> get(const std::string &logger) {
  std::shared_ptr<spdlog::logger> spdlog_logger = spdlog::get(logger);
  if (!spdlog_logger) {
    if (dist_sink) {
      spdlog_logger = std::make_shared<spdlog::logger>(logger, dist_sink);
    } else {
      spdlog_logger = std::make_shared<spdlog::logger>(logger);
    }
    spdlog_logger->set_level(spdlog::level::level_enum::trace);
    spdlog::register_logger(spdlog_logger);
    specula::logging::configure_logging();
  }
  return spdlog_logger;
}

} // namespace specula::logging

#endif // SPECULA_UTIL_LOG_HPP
