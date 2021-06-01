#ifndef SPECULA_CLI_EXIT_CODE_HPP
#define SPECULA_CLI_EXIT_CODE_HPP

namespace specula::cli {
enum ExitCode {
  OK = 0,
  OK_EXIT = 1,
  ARGPARSE_ERROR,
  LOG_INIT_FAILURE
};
}

#endif
