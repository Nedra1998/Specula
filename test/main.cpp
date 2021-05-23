#include <iostream>
#include <memory>

#include <catch2/catch_session.hpp>
#include <catch2/internal/catch_compiler_capabilities.hpp>
#include <catch2/internal/catch_leak_detector.hpp>
#include <catch2/internal/catch_platform.hpp>

#include <spdlog/common.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <specula/util/log.hpp>

#include "test_sink.hpp"

namespace Catch {
CATCH_INTERNAL_START_WARNINGS_SUPPRESSION
CATCH_INTERNAL_SUPPRESS_GLOBALS_WARNINGS
LeakDetector leakDetector;
CATCH_INTERNAL_STOP_WARNINGS_SUPPRESSION
} // namespace Catch

int main(int argc, char *argv[]) {

  try {
    specula::logging::dist_sink =
        std::make_shared<spdlog::sinks::dist_sink_mt>();
    {
      auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      sink->set_level(spdlog::level::trace);
      specula::logging::dist_sink->add_sink(sink);
    }
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    return 1;
  }

  (void)&Catch::leakDetector;

  return Catch::Session().run(argc, argv);
}
