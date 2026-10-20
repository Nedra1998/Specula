#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>
#include <fmt/std.h>

#include "specula/options.hpp"
#include "specula/specula.hpp"

int main(int argc, char *argv[]) {
  Catch::Session session;
  int returnCode = session.applyCommandLine(argc, argv);
  if (returnCode != 0) {
    return returnCode;
  }

  specula::initialize(specula::SpeculaOptions{});
  int result = session.run();
  specula::cleanup();

  return result;
}
