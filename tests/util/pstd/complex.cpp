#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/pstd.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Complex", "[util][pstd]") {
  SECTION("Default Constructors") {
    pstd::complex<float> c1(5.0f);
    CHECK_THAT(c1.re, WithinAbs(5.0f, 0.0001f));
    CHECK_THAT(c1.im, WithinAbs(0.0f, 0.0001f));

    pstd::complex<double> c2(3.0, 4.0);
    CHECK_THAT(c2.re, WithinAbs(3.0, 0.0001f));
    CHECK_THAT(c2.im, WithinAbs(4.0, 0.0001f));
  }

  SECTION("Arithmetic Operations with another complex number") {
    pstd::complex<float> c1(1.0f, 2.0f);
    pstd::complex<float> c2(2.0f, -3.0f);
    // Addition
    auto c_add = c1 + c2;
    CHECK_THAT(c_add.re, WithinAbs(3.0f, 0.0001f));
    CHECK_THAT(c_add.im, WithinAbs(-1.0f, 0.0001f));
    // Subtraction
    auto c_sub = c1 - c2;
    CHECK_THAT(c_sub.re, WithinAbs(-1.0f, 0.0001f));
    CHECK_THAT(c_sub.im, WithinAbs(5.0f, 0.0001f));
    // Multiplication
    auto c_mul = c1 * c2;
    CHECK_THAT(c_mul.re, WithinAbs(8.0f, 0.0001f));
    CHECK_THAT(c_mul.im, WithinAbs(1.0f, 0.0001f));
    // Division
    auto c_div = c1 / c2;
    CHECK_THAT(c_div.re, WithinAbs(-0.3076923f, 0.0001f));
    CHECK_THAT(c_div.im, WithinAbs(0.5384615f, 0.0001f));
  }

  SECTION("Arithmetic Operations with scalar values") {
    pstd::complex<double> c1(2.5, -1.5);
    // Addition
    auto c_add = c1 + 3.0;
    CHECK_THAT(c_add.re, WithinAbs(5.5, 0.0001f));
    CHECK_THAT(c_add.im, WithinAbs(-1.5, 0.0001f));
    // Subtraction
    auto c_sub = c1 - 1.0;
    CHECK_THAT(c_sub.re, WithinAbs(1.5, 0.0001f));
    CHECK_THAT(c_sub.im, WithinAbs(-1.5, 0.0001f));
    // Multiplication
    auto c_mul = c1 * 2.0;
    CHECK_THAT(c_mul.re, WithinAbs(5.0, 0.0001f));
    CHECK_THAT(c_mul.im, WithinAbs(-3.0, 0.0001f));
    // Division
    auto c_div = c1 / 2.0;
    CHECK_THAT(c_div.re, WithinAbs(1.25, 0.0001f));
    CHECK_THAT(c_div.im, WithinAbs(-0.75, 0.0001f));
  }

  SECTION("Utility functions") {
    pstd::complex<double> c1(3.0, 4.0);
    // Real part
    CHECK_THAT(pstd::real(c1), WithinAbs(3.0, 0.0001f));
    // Imaginary part
    CHECK_THAT(pstd::imag(c1), WithinAbs(4.0, 0.0001f));
    // Norm
    CHECK_THAT(pstd::norm(c1), WithinAbs(25.0, 0.0001f));
    // Absolute value (magnitude)
    CHECK_THAT(pstd::abs(c1), WithinAbs(5.0, 0.0001f));
    // Square root
    auto c_sqrt = pstd::sqrt(c1);
    CHECK_THAT(c_sqrt.re, WithinAbs(2.0, 0.0001f));
    CHECK_THAT(c_sqrt.im, WithinAbs(1.0, 0.0001f));
  }
}
