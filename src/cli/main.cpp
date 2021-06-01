#include <map>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Validators.hpp>
#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#if defined(_WIN32) || defined(_WIN64)
#  define NOMINMAX
#  include <io.h>
#  include <windows.h>
#  undef min
#  undef max
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
#  include <unistd.h>
#endif

#include <specula/util/log.hpp>
#include <specula/version.hpp>

#include "cmd/version.hpp"
#include "exit_code.hpp"

using namespace specula::cli;

enum LogColorMode { AUTO, ALWAYS, NEVER };

static const std::map<std::string, LogColorMode> log_color_map{
    {"auto", LogColorMode::AUTO},
    {"always", LogColorMode::ALWAYS},
    {"never", LogColorMode::NEVER}};
static const std::map<std::string, spdlog::level::level_enum> log_level_map{
    {"trace", spdlog::level::trace}, {"debug", spdlog::level::debug},
    {"info", spdlog::level::info},   {"warn", spdlog::level::warn},
    {"err", spdlog::level::err},     {"critical", spdlog::level::critical},
    {"off", spdlog::level::off},
};

int main(int argc, const char **argv) {
  CLI::App app{fmt::format("Specula CLI v{}", specula::version::core)};

  bool brief_version = false;
  LogColorMode log_color = LogColorMode::AUTO;
#ifdef NDEBUG
  spdlog::level::level_enum log_level = spdlog::level::info;
#else
  spdlog::level::level_enum log_level = spdlog::level::debug;
#endif
  app.add_option("-v,--verbosity", log_level, "Set log verbosity", true)
      ->transform(CLI::CheckedTransformer(log_level_map, CLI::ignore_case))
      ->group("Logging");
  app.add_option("-c,--color", log_color, "Configure log coloring", true)
      ->transform(CLI::CheckedTransformer(log_color_map, CLI::ignore_case))
      ->group("Logging");
  app.add_flag("--version", brief_version, "Display brief version info");

  CLI::App *version_cmd = version::options(&app);

  try {
    app.parse(argc, argv);
  } catch (CLI::ParseError &e) {
    app.exit(e);
    return ARGPARSE_ERROR;
  }

  if (brief_version) {
    fmt::print("Specula {}\n", specula::version::core);
    return OK;
  }

  try {
    specula::logging::dist_sink =
        std::make_shared<spdlog::sinks::dist_sink_mt>();
    {
#if defined(_WIN32) || defined(_WIN64)
      bool is_tty = _isatty(_fileno(stdout));
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
      bool is_tty = ::isatty(fileno(stdout)) != 0;
#else
      bool is_tty = false;
#endif
      if (log_color == LogColorMode::ALWAYS ||
          (is_tty && log_color == LogColorMode::AUTO)) {
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink->set_level(log_level);
        specula::logging::dist_sink->add_sink(sink);
      } else {
        auto sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        sink->set_level(log_level);
        specula::logging::dist_sink->add_sink(sink);
      }
    }
  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    return ExitCode::LOG_INIT_FAILURE;
  }

  LINFO("cli", "Specula v{}", specula::version::semver);

  ExitCode ret = OK;

  if (ret == OK && app.got_subcommand(version_cmd))
    ret = version::main();

  return ret;
}
