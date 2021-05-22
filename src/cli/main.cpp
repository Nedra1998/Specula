#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <fmt/format.h>

int main(int argc, const char **argv) {
  CLI::App app{fmt::format("Specula CLI v{}", "0.1.0")};

  try {
    app.parse(argc, argv);
  } catch (CLI::ParseError &e) {
    return app.exit(e);
  }
}
