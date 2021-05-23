#include "util/log.hpp"
#include <mutex>

namespace specula::logging {
std::shared_ptr<spdlog::sinks::dist_sink_mt> dist_sink = nullptr;
} // namespace specula::logging

static std::once_flag _configure_logging_impl;
void configure_logging_impl() {
#ifdef NDEBUG
  spdlog::set_level(spdlog::level::debug);
#else
  spdlog::set_level(spdlog::level::trace);
#endif
  spdlog::set_pattern(
      "[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [%s:%!:%#] <%t> %v");
}
void specula::logging::configure_logging() {
  std::call_once(_configure_logging_impl, configure_logging_impl);
}
