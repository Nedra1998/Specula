#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <fmt/format.h>
#include <spdlog/common.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <specula/util/log.hpp>
#include <specula/version.hpp>

int main(int argc, const char **argv) {
  CLI::App app{fmt::format("Specula CLI v{}", specula::version::core)};

  try {
    app.parse(argc, argv);
  } catch (CLI::ParseError &e) {
    return app.exit(e);
  }

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

  LINFO("cli", "Specula v{}", specula::version::semver);
}
