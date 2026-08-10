/**
 * @file log.hpp
 * @brief Common logging utilities used by the renderer
 *
 * This file provides a small wrapper around the logging utilities that are provided by
 * [spdlog](https://github.com/gabime/spdlog). It primarily consists of some macros which map
 * directly to spdlog logging macros, and provides an initialization function for clients to seutp
 * the logging interface.
 */
#ifndef SPECULA_UTIL_LOG_HPP
#define SPECULA_UTIL_LOG_HPP

#include <vector>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

/// @brief Log a trace message using the default logger
#define LOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)

/// @brief Log a debug message using the default logger
#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)

/// @brief Log an info message using the default logger
#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)

/// @brief Log a warning message using the default logger
#define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)

/// @brief Log an error message using the default logger
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)

/// @brief Log a critical error message using the default logger
#define LOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

/// @brief Log a trace message using the provided logger
#define LOGGER_TRACE(...) SPDLOG_LOGGER_TRACE(__VA_ARGS__)

/// @brief Log a debug message using the provided logger
#define LOGGER_DEBUG(...) SPDLOG_LOGGER_DEBUG(__VA_ARGS__)

/// @brief Log an info message using the provided logger
#define LOGGER_INFO(...) SPDLOG_LOGGER_INFO(__VA_ARGS__)

/// @brief Log a warning message using the provided logger
#define LOGGER_WARN(...) SPDLOG_LOGGER_WARN(__VA_ARGS__)

/// @brief Log an error message using the provided logger
#define LOGGER_ERROR(...) SPDLOG_LOGGER_ERROR(__VA_ARGS__)

/// @brief Log a critical error message using the provided logger
#define LOGGER_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(__VA_ARGS__)

#define LOG_STACKTRACE()                                                                           \
  do {                                                                                             \
    const auto stack_trace = specula::logging::fmt_stack_trace();                                  \
    if (!stack_trace.empty()) {                                                                    \
      LOG_CRITICAL("{}\n", stack_trace);                                                           \
    }                                                                                              \
  } while (false)

// NOLINTEND(cppcoreguidelines-macro-usage)

/**
 * @brief Specula logging utilities
 *
 * This namespace contains all of the utility methods that are involved with logging within the
 * renderer. All of the actualy logging logic is provided by
 * [spdlog](https://github.com/gabime/spdlog). This namespace just provides some additional tooling
 * arround initializing and using spdlog within the renderer.
 */
namespace specula::logging {
  /**
   * @brief Create the default logger used by the renderer
   *
   * This method creates the default `specula` logger, and configures it using a distribute log
   * sink, distributing the log to the provided `sinks`. So it should be called before any other
   * operations in the renderer so that the logging is available.
   *
   * If the renderer has been compiled with `ENABLE_PROFILING` then this also instantiates a tracy
   * log sink, which will automatically send log messages to tracy. The tracy log sink is configured
   * to send log messages up to and including debug logs, but will not send trace log messages.
   *
   * @param sinks An optional list of additional sinks to add to the logger
   * @return `true` if the logger was successfully initialized, `false` otherwise
   */
  bool initialize(const std::vector<spdlog::sink_ptr> &sinks = {}, bool color = false);

  std::string fmt_stack_trace();
} // namespace specula::logging

#endif // SPECULA_UTIL_LOG_HPP
