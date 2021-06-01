#ifndef SPECULA_CLI_CMD_VERSION_HPP
#define SPECULA_CLI_CMD_VERSION_HPP

#include <CLI/App.hpp>

#include "exit_code.hpp"

namespace specula::cli::version {
CLI::App* options(CLI::App *app);
ExitCode main();
}

#endif
