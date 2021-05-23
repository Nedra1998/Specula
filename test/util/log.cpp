#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "test_sink.hpp"
#include <specula/util/log.hpp>

using namespace specula;

TEST_CASE("Debug sink logs messages", "[util][debug]") {
  auto sink = get_test_sink();
  LTRACE("debug", "Test trace log");
  LDEBUG("debug", "Test debug log");
  LINFO("debug", "Test info log");
  LWARN("debug", "Test warn log");
  LERR("debug", "Test err log");
  LCRITICAL("debug", "Test critical log");

  REQUIRE(sink->get("debug", TRACE).size() == 1);
  REQUIRE(sink->get("debug", DEBUG).size() == 1);
  REQUIRE(sink->get("debug", INFO).size() == 1);
  REQUIRE(sink->get("debug", WARN).size() == 1);
  REQUIRE(sink->get("debug", ERR).size() == 1);
  REQUIRE(sink->get("debug", CRITICAL).size() == 1);
  REQUIRE(sink->get("debug", ALL).size() == 6);
  remove_test_sink(sink);
}

TEST_CASE("Debug sink searches log messages", "[util][debug]") {
  auto sink = get_test_sink();
  LTRACE("debug", "Test trace log 1");
  LDEBUG("debug", "Test debug log 2");
  LTRACE("debug", "Test trace log 3");
  LTRACE("debug", "Test trace log 4");

  REQUIRE(sink->find("trace", "debug").size() == 3);
  REQUIRE(sink->find("log 2", "debug").size() == 1);
  REQUIRE(sink->find("hello world", "debug").size() == 0);
  remove_test_sink(sink);
}
