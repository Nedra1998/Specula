#include "util/log.hpp"

#include <iostream>
#include <mutex>
#include <vector>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/spdlog.h>
#include <tracy/Tracy.hpp>

template <typename Mutex> class tracy_sink : public spdlog::sinks::base_sink<Mutex> {
protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    std::string formatted_string = fmt::to_string(formatted);
    switch (msg.level) {
    case spdlog::level::critical:
      TracyMessageC(formatted_string.c_str(), formatted_string.size(), 0xf38ba8);
      break;
    case spdlog::level::err:
      TracyMessageC(formatted_string.c_str(), formatted_string.size(), 0xfab387);
      break;
    case spdlog::level::warn:
      TracyMessageC(formatted_string.c_str(), formatted_string.size(), 0xf9e2af);
      break;
    case spdlog::level::info:
      TracyMessageC(formatted_string.c_str(), formatted_string.size(), 0xa6e3a1);
      break;
    case spdlog::level::debug:
      TracyMessageC(formatted_string.c_str(), formatted_string.size(), 0x74c7ec);
      break;
    case spdlog::level::trace:
      TracyMessageC(formatted_string.c_str(), formatted_string.size(), 0xcba6f7);
      break;

    default:
      break;
    }
  }

  void flush_() override { std::cout << std::flush; }
};

using tracy_sink_mt = tracy_sink<std::mutex>;
using tracy_sink_st = tracy_sink<spdlog::details::null_mutex>;

bool specula::logging::initialize(std::vector<spdlog::sink_ptr> sinks) {
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
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    return false;
  }
}