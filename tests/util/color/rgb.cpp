// WARNING: This file was generated using AI and may contain inaccuracies or errors.
// It should be reviewed and tested thoroughly before use in production.

#include "specula/util/color/rgb.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Rgb color operations", "[util][color]") {
  SECTION("Basic construction and component access") {
    Rgb c1;
    CHECK(c1.r == 0.0f);
    CHECK(c1.g == 0.0f);
    CHECK(c1.b == 0.0f);

    Rgb c2(1.0f, 2.0f, 3.0f);
    CHECK(c2.r == 1.0f);
    CHECK(c2.g == 2.0f);
    CHECK(c2.b == 3.0f);

    CHECK(c2[0] == 1.0f);
    CHECK(c2[1] == 2.0f);
    CHECK(c2[2] == 3.0f);

    c2[0] = 4.0f;
    c2[1] = 5.0f;
    c2[2] = 6.0f;
    CHECK(c2.r == 4.0f);
    CHECK(c2.g == 5.0f);
    CHECK(c2.b == 6.0f);
  }

  SECTION("Average computation") {
    Rgb c1(3.0f, 6.0f, 9.0f);
    CHECK_THAT(c1.average(), WithinAbs(6.0f, 0.0001f));

    Rgb c2(1.5f, 2.5f, 3.5f);
    CHECK_THAT(c2.average(), WithinAbs(2.5f, 0.0001f));
  }

  SECTION("Addition operations") {
    Rgb c1(1.0f, 2.0f, 3.0f);
    Rgb c2(4.0f, 5.0f, 6.0f);

    // Test operator+
    Rgb sum = c1 + c2;
    CHECK(sum.r == 5.0f);
    CHECK(sum.g == 7.0f);
    CHECK(sum.b == 9.0f);

    // Test operator+=
    Rgb c3 = c1;
    c3 += c2;
    CHECK(c3.r == 5.0f);
    CHECK(c3.g == 7.0f);
    CHECK(c3.b == 9.0f);
  }

  SECTION("Subtraction operations") {
    Rgb c1(10.0f, 20.0f, 30.0f);
    Rgb c2(1.0f, 2.0f, 3.0f);

    // Test operator-
    Rgb diff = c1 - c2;
    CHECK(diff.r == 9.0f);
    CHECK(diff.g == 18.0f);
    CHECK(diff.b == 27.0f);

    // Test operator-=
    Rgb c3 = c1;
    c3 -= c2;
    CHECK(c3.r == 9.0f);
    CHECK(c3.g == 18.0f);
    CHECK(c3.b == 27.0f);

    // Test scalar - Rgb
    Rgb result = 5.0f - c2;
    CHECK(result.r == 4.0f);
    CHECK(result.g == 3.0f);
    CHECK(result.b == 2.0f);
  }

  SECTION("Multiplication operations") {
    Rgb c1(2.0f, 3.0f, 4.0f);
    Rgb c2(3.0f, 4.0f, 5.0f);

    // Test Rgb * Rgb
    Rgb product = c1 * c2;
    CHECK(product.r == 6.0f);
    CHECK(product.g == 12.0f);
    CHECK(product.b == 20.0f);

    // Test Rgb *= Rgb
    Rgb c3 = c1;
    c3 *= c2;
    CHECK(c3.r == 6.0f);
    CHECK(c3.g == 12.0f);
    CHECK(c3.b == 20.0f);

    // Test Rgb * scalar
    Rgb scaled = c1 * 2.0f;
    CHECK(scaled.r == 4.0f);
    CHECK(scaled.g == 6.0f);
    CHECK(scaled.b == 8.0f);

    // Test scalar * Rgb
    Rgb scaled2 = 3.0f * c1;
    CHECK(scaled2.r == 6.0f);
    CHECK(scaled2.g == 9.0f);
    CHECK(scaled2.b == 12.0f);

    // Test Rgb *= scalar
    Rgb c4 = c1;
    c4 *= 2.0f;
    CHECK(c4.r == 4.0f);
    CHECK(c4.g == 6.0f);
    CHECK(c4.b == 8.0f);
  }

  SECTION("Division operations") {
    Rgb c1(6.0f, 8.0f, 10.0f);
    Rgb c2(2.0f, 4.0f, 5.0f);

    // Test Rgb / Rgb
    Rgb quotient = c1 / c2;
    CHECK(quotient.r == 3.0f);
    CHECK(quotient.g == 2.0f);
    CHECK(quotient.b == 2.0f);

    // Test Rgb /= Rgb
    Rgb c3 = c1;
    c3 /= c2;
    CHECK(c3.r == 3.0f);
    CHECK(c3.g == 2.0f);
    CHECK(c3.b == 2.0f);

    // Test Rgb / scalar
    Rgb scaled = c1 / 2.0f;
    CHECK(scaled.r == 3.0f);
    CHECK(scaled.g == 4.0f);
    CHECK(scaled.b == 5.0f);

    // Test Rgb /= scalar
    Rgb c4 = c1;
    c4 /= 2.0f;
    CHECK(c4.r == 3.0f);
    CHECK(c4.g == 4.0f);
    CHECK(c4.b == 5.0f);
  }

  SECTION("Unary negation") {
    Rgb c(1.0f, -2.0f, 3.0f);
    Rgb neg = -c;
    CHECK(neg.r == -1.0f);
    CHECK(neg.g == 2.0f);
    CHECK(neg.b == -3.0f);
  }

  SECTION("Equality and inequality operators") {
    Rgb c1(1.0f, 2.0f, 3.0f);
    Rgb c2(1.0f, 2.0f, 3.0f);
    Rgb c3(3.0f, 2.0f, 1.0f);

    CHECK(c1 == c2);
    CHECK(c1 != c3);
    CHECK(c2 != c3);
  }

  SECTION("Utility functions") {
    SECTION("max") {
      Rgb c1(1.0f, 5.0f, 3.0f);
      Rgb c2(4.0f, 2.0f, 6.0f);

      Rgb result = max(c1, c2);
      CHECK(result.r == 4.0f);
      CHECK(result.g == 5.0f);
      CHECK(result.b == 6.0f);
    }

    SECTION("lerp") {
      Rgb c1(0.0f, 10.0f, 20.0f);
      Rgb c2(10.0f, 20.0f, 30.0f);

      // Test t=0 (should be c1)
      Rgb result1 = lerp(0.0f, c1, c2);
      CHECK(result1.r == c1.r);
      CHECK(result1.g == c1.g);
      CHECK(result1.b == c1.b);

      // Test t=1 (should be c2)
      Rgb result2 = lerp(1.0f, c1, c2);
      CHECK(result2.r == c2.r);
      CHECK(result2.g == c2.g);
      CHECK(result2.b == c2.b);

      // Test t=0.5 (should be midpoint)
      Rgb result3 = lerp(0.5f, c1, c2);
      CHECK(result3.r == 5.0f);
      CHECK(result3.g == 15.0f);
      CHECK(result3.b == 25.0f);
    }

    SECTION("clamp") {
      Rgb c(-1.0f, 0.5f, 2.0f);

      Rgb result = clamp(c, 0.0f, 1.0f);
      CHECK(result.r == 0.0f);
      CHECK(result.g == 0.5f);
      CHECK(result.b == 1.0f);
    }

    SECTION("clamp_zero") {
      Rgb c(-1.0f, 0.0f, 2.0f);

      Rgb result = clamp_zero(c);
      CHECK(result.r == 0.0f);
      CHECK(result.g == 0.0f);
      CHECK(result.b == 2.0f);
    }
  }
}
