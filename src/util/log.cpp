#include "specula/util/log.hpp"

#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <cpptrace/cpptrace.hpp>
#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

namespace specula::logging {
  /**
   * @brief A spdlog logger sink for sending log messages to tracy
   *
   * This is a specialized logger for integrating [spdlog](https://github.com/gabime/spdlog) with
   * [Tracy](https://github.com/wolfpld/tracy), allowing the inspection and search of log messages
   * to happen directly in tracy along with the other profiling information.
   *
   * @tparam Mutex The mutex type to use for handling multi-threaded logging requests
   */
  template <typename Mutex> class TracySink : public spdlog::sinks::base_sink<Mutex> {
  protected:
    /**
     * @brief Log a message to the sink
     *
     * This formats the log message using the configured formatter, and then sends
     * the message to Tracy using `TracyMessageC`. For visual convinience this
     * method also colorizes the logs that it sends to Tracy based on the log
     * level.
     *
     * @param msg The spdlog message to write to Tracy
     */
    void sink_it_(const spdlog::details::log_msg &msg) override {
      spdlog::memory_buf_t formatted;
      spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

      // NOLINTNEXTLINE(bugprone-unused-local-non-trivial-variable)
      std::string formatted_string = fmt::to_string(formatted);

      uint32_t color = 0xffffff;
      switch (msg.level) {
      case spdlog::level::critical:
        color = 0xf38ba8;
        break;
      case spdlog::level::err:
        color = 0xfab387;
        break;
      case spdlog::level::warn:
        color = 0xf9e2af;
        break;
      case spdlog::level::info:
        color = 0xa6e3a1;
        break;
      case spdlog::level::debug:
        color = 0x74c7ec;
        break;
      case spdlog::level::trace:
        color = 0xcba6f7;
        break;
      default:
        break;
      }
      TracyMessageC(formatted_string.c_str(), formatted_string.size(), color);
    }

    /**
     * @brief Flush logs buffered in the sink
     *
     * This is a no-op method, that is required to be defined from the spdlog base class. It is not
     * required as the messages are always sent to tracy, there is no need to flushing any internal
     * buffers.
     */
    void flush_() override {}
  };
} // namespace specula::logging

using tracy_sink_mt = specula::logging::TracySink<std::mutex>;
using tracy_sink_st = specula::logging::TracySink<spdlog::details::null_mutex>;

static bool use_colors = false;

bool specula::logging::initialize(const std::vector<spdlog::sink_ptr> &sinks, bool color) {
  use_colors = color;
  try {
    auto sink = std::make_shared<spdlog::sinks::dist_sink_mt>(sinks);
    sink->set_level(spdlog::level::trace);

#ifdef TRACY_ENABLE
    auto tracy_sink = std::make_shared<tracy_sink_mt>();
    tracy_sink->set_pattern("[%l] %v");
    tracy_sink->set_level(spdlog::level::debug);
    sink->add_sink(tracy_sink);
#endif

    auto logger = std::make_shared<spdlog::logger>("specula", sink);
    logger->set_level(spdlog::level::trace);
    spdlog::set_default_logger(logger);
    return true;
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << '\n';
    return false;
  }
}

std::string specula::logging::fmt_stack_trace() {
  const char *env = std::getenv("SPECULA_BACKTRACE");
  if (env != nullptr && std::string_view(env) == "0") {
    return {};
  }

  const auto &cpptrace = cpptrace::generate_trace();
  std::stringstream out;
  if (env == nullptr || std::string_view(env) == "1") {
    cpptrace.print(out, use_colors);
  } else if (std::string_view(env) == "full") {
    cpptrace.print_with_snippets(out, use_colors);
  }

  return out.str();
}
