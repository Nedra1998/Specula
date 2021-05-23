#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <specula/util/log.hpp>
#include "test_sink.hpp"

using namespace specula;

TEST_CASE("Logs are written", "[util]") {
  LTRACE("debug", "Test trace log");
  LDEBUG("debug", "Test debug log");
  LINFO("debug", "Test info log");
  LWARN("debug", "Test warn log");
  LERR("debug", "Test err log");
  LCRITICAL("debug", "Test critical log");

  REQUIRE(logger->get_trace("test").size() == 1);
  REQUIRE(logger->get_debug("test").size() == 1);
  REQUIRE(logger->get_info("test").size() == 1);
  REQUIRE(logger->get_warn("test").size() == 1);
  REQUIRE(logger->get_err("test").size() == 1);
  REQUIRE(logger->get_critical("test").size() == 1);
}

TEST_CASE("Test sink filters correctly", "[util]") {
  LTRACE("a", "Test trace log");
  LDEBUG("a", "Test trace log");
  LTRACE("c", "Test trace log");
  LTRACE("c", "Test trace log");

  REQUIRE(logger->get_trace("a").size() == 1);
  REQUIRE(logger->get_trace("b").size() == 0);
  REQUIRE(logger->get_trace("c").size() == 2);
}
