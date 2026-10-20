#include "specula/util/color/color_encoding.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("sRGB", "[util][color]") {
  SECTION("Conversion") {
    SECTION("sRGB linear and 8-bit") {
      for (size_t i = 0; i < 256; ++i) {
        Float x = srgb_to_linear(i * (1.0f / 255.0f));
        Float y = srgb8_to_linear(i);
        CHECK_THAT(x, WithinAbs(y, 1e-5f));
      }
    }

    SECTION("Round trip sRGB to Linear") {
      for (size_t i = 0; i < 256; ++i) {
        Float x = srgb_to_linear(i * (1.0f / 255.0f));
        Float y = linear_to_srgb(x) * 255.0f;
        CHECK_THAT(y, WithinAbs(i, 1e-4f));
      }
    }

    SECTION("Round trip Linear to sRGB") {
      for (size_t i = 0; i < 256; ++i) {
        Float x = linear_to_srgb(i * (1.0f / 255.0f));
        Float y = srgb_to_linear(x) * 255.0f;
        CHECK_THAT(y, WithinAbs(i, 3e-4f));
      }
    }
  }
}
