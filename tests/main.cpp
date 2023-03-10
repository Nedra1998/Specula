#include <iostream>
#include <memory>

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <specula/util/log.hpp>

#include "globals.hpp"

#if defined(_WIN32) || defined(_WIN64)
#  include <io.h>
#  include <windows.h>
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
#  include <unistd.h>
#endif

std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> log_ringbuffer = nullptr;

int main(int argc, char *argv[]) {

  Catch::Session session;

  int returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0)
    return returnCode;

  bool use_color = false;
  switch (session.config().defaultColourMode()) {
  case Catch::ColourMode::ANSI:
    use_color = true;
    break;
  case Catch::ColourMode::PlatformDefault:
#if defined(_WIN32) || defined(_WIN64)
    use_color = _isatty(_fileno(stdout));
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    use_color = ::isatty(fileno(stdout)) != 0;
#else
    use_color = false;
#endif
    break;
  default:
    use_color = false;
    break;
  }

  log_ringbuffer = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(256);
  log_ringbuffer->set_level(spdlog::level::trace);
  log_ringbuffer->set_pattern("%l %v");

  std::shared_ptr<spdlog::sinks::sink> stdout_sink = nullptr;
  if (use_color)
    stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  else
    stdout_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();

  switch (session.config().verbosity()) {
  case Catch::Verbosity::Quiet:
    stdout_sink->set_level(spdlog::level::err);
    break;
  case Catch::Verbosity::Normal:
    stdout_sink->set_level(spdlog::level::info);
    break;
  case Catch::Verbosity::High:
    stdout_sink->set_level(spdlog::level::trace);
    break;
  }

  if (use_color)
    stdout_sink->set_pattern(
        "\033[90m[%H:%M:%S.%e]\033[0m \033[1m%^%-8l%$\033[0m \033[36m(%s:%#)\033[0m %v");
  else
    stdout_sink->set_pattern("[%H:%M:%S.%e] %-8l (%s:%#) %v");

  if (!specula::logging::initialize({stdout_sink, log_ringbuffer}))
    return -1;

  return session.run();
}

uint32_t factorial(uint32_t number) {
  return number <= 1 ? number : factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]") {
  REQUIRE(factorial(1) == 1);
  REQUIRE(factorial(2) == 2);
  REQUIRE(factorial(3) == 6);
  REQUIRE(factorial(10) == 3'628'800);
}