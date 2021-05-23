#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <specula/version.hpp>

using namespace specula;

TEST_CASE("Version variables are defined") {
  REQUIRE(!version::semver.empty());
  REQUIRE(!version::core.empty());
  REQUIRE(version::major >= 0);
  REQUIRE(version::minor >= 0);
  REQUIRE(version::patch >= 0);
  REQUIRE(version::major < std::numeric_limits<unsigned int>::max());
  REQUIRE(version::minor < std::numeric_limits<unsigned int>::max());
  REQUIRE(version::patch < std::numeric_limits<unsigned int>::max());
}

TEST_CASE("Version is valid semver") {
  REQUIRE_THAT(
      version::semver,
      Catch::Matchers::Matches(
          "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)(?:-((?:0|[1-9]\\d*|"
          "\\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\\.(?:0|[1-9]\\d*|\\d*[a-zA-Z-][0-9a-"
          "zA-Z-]*))*))?(?:\\+([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$"));
  REQUIRE_THAT(version::core,
               Catch::Matchers::Matches(
                   "^(0|[1-9]\\d*)\\.(0|[1-9]\\d*)\\.(0|[1-9]\\d*)$"));
  REQUIRE_THAT(
      version::build,
      Catch::Matchers::Matches("^(?:([0-9a-zA-Z-]+(?:\\.[0-9a-zA-Z-]+)*))?$"));
}
