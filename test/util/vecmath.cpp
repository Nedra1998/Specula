#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <fmt/format.h>

#include <specula/util/vecmath.hpp>

using namespace specula;

TEMPLATE_TEST_CASE("Quaternion", "[vecmath][util]", int, float, double) {
  Quaternion<TestType> q(-1, 10, 2, -5);

  SECTION("constructor") {
    REQUIRE(q.x == -1);
    REQUIRE(q.y == 10);
    REQUIRE(q.z == 2);
    REQUIRE(q.w == -5);
  }

  SECTION("comparison operators") {
    REQUIRE(q == Quaternion<TestType>(-1, 10, 2, -5));
    REQUIRE(q != Quaternion<TestType>(-1, 100, 2, -5));
  }

  SECTION("binary operators") {
    REQUIRE(q + q == Quaternion<TestType>(-2, 20, 4, -10));
    REQUIRE(q - q == Quaternion<TestType>(0, 0, 0, 0));
    REQUIRE(q * 2 == Quaternion<TestType>(-2, 20, 4, -10));
    REQUIRE(2 * q == Quaternion<TestType>(-2, 20, 4, -10));
    REQUIRE(q / 2 == Quaternion<TestType>(static_cast<TestType>(-0.5), 5, 1,
                                          static_cast<TestType>(-2.5)));
  }

  SECTION("unary operators") {
    REQUIRE(-q == Quaternion<TestType>(1, -10, -2, 5));
  }

  SECTION("formatting") {
    REQUIRE(q.to_string() ==
            fmt::format("[ {}, {}, {}, {} ]", static_cast<TestType>(-1),
                        static_cast<TestType>(10), static_cast<TestType>(2),
                        static_cast<TestType>(-5)));
  }
}
