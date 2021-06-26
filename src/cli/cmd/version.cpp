#include "version.hpp"

#include <CLI/App.hpp>
#include <fmt/format.h>
#include <specula/util/log.hpp>
#include <specula/version.hpp>

#include "exit_code.hpp"

using namespace specula::cli;

CLI::App *specula::cli::version::options(CLI::App *app) {
  CLI::App *cmd =
      app->add_subcommand("version", "Display detailed verion information");
  return cmd;
}
ExitCode specula::cli::version::main() {
  fmt::print("Specula Version: {}\n", specula::version::semver);

  fmt::print("  C++ Version:      C++{}.{} ({})\n",
             ((__cplusplus / 100L) % 100), (__cplusplus % 100), __cplusplus);

#if defined(__clang__) && defined(__apple_build_version)
  fmt::print("  Compiler:         AppleClang v{}.{}.{}\n", __clang_major__,
             __clang_minor__, __clang_patchlevel__);
#elif defined(__clang__)
  fmt::print("  Compiler:         Clang v{}.{}.{}\n", __clang_major__,
             __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
#  if defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
  fmt::print("  Compiler:         GNU v{}.{}.{}\n", __GNUC__, __GNUC_MINOR__,
             __GNUC_PATCHLEVEL__);
#  elif defined(__GNUC_MINOR__)
  fmt::print("  Compiler:         GNU v{}.{}\n", __GNUC__, __GNUC_MINOR__);
#  else
  fmt::print("  Compiler:         GNU v{}\n", __GNUC__);
#  endif
#elif defined(_MSC_VER)
#  if defined(_MSC_FULL_VER)
#    if _MSC_VER >= 1400
  fmt::print("  Compiler:         MSVC v{}.{}.{}\n", _MSC_VER / 100,
             _MSC_VER % 100, _MSC_FULL_VER % 100000);
#    else
  fmt::print("  Compiler:         MSVC v{}.{}.{}\n", _MSC_VER / 100,
             _MSC_VER % 100, _MSC_FULL_VER % 10000);
#    endif
#  else
  fmt::print("  Compiler:         MSVC v{}.{}\n", _MSC_VER / 100,
             _MSC_VER % 100);
#  endif
#else
  fmt::print("  Compiler:         Unknown\n");
#endif

#if defined(__TIMESTAMP__)
  fmt::print("  Date Time:        {}\n", __TIMESTAMP__);
#else
  fmt::print("  Date Time:        {} {}\n", __DATE__, __TIME__);
#endif

#if defined(_WIN32) || defined(_WIN64)
  fmt::print("  Operating System: Windows\n");
#elif defined(__APPLE__)
  fmt::print("  Operating System: MacOS\n");
#elif defined(__unix__) || defined(__unix)
  fmt::print("  Operating System: Linux\n");
#else
  fmt::print("  Operating System: Unknown\n");
#endif

  return OK;
}
