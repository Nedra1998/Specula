#include "specula/util/check.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "globals.hpp"

using namespace specula;

TEST_CASE("Stack trace generation", "[util][check]") {
  print_stack_trace(true);

  std::vector<std::string> log_messages = log_ringbuffer->last_formatted();
  CHECK_FALSE(log_messages.empty());
  CHECK_THAT(log_messages[0], Catch::Matchers::ContainsSubstring("print_stack_trace"));
}
