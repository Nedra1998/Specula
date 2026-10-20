#include "specula/util/spectrum/rgb_unbounded_spectrum.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/color.hpp"
#include "specula/util/colorspace.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/spectrum.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("RgbUnboundedSpectrum", "[util][spectrum]") {
  SECTION("Small values") {
    Rgb rgb(0.00010678071, 0, 0.000010491596);
    RgbUnboundedSpectrum rs(*RgbColorSpace::SRGB, rgb);

    for (int lambda = 360; lambda < 840; ++lambda) {
      CHECK(rs(lambda) < 0.05f);
    }
  }

  SECTION("Max value") {
    Rng rng;
    const auto cs =
        GENERATE(*RgbColorSpace::ACES2065_1, *RgbColorSpace::REC_2020, *RgbColorSpace::SRGB);
    for (size_t i = 0; i < 100; ++i) {
      Rgb rgb(rng.uniform<Float>(), rng.uniform<Float>(), rng.uniform<Float>());
      rgb *= 10.0f;
      RgbUnboundedSpectrum rs(cs, rgb);

      Float m = rs.max_value();
      Float sm = 0;
      for (Float lambda = LAMBDA_MIN; lambda <= LAMBDA_MAX; lambda += 1.0 / 16.0) {
        sm = std::max(sm, rs(lambda));
      }

      CHECK_THAT(m, WithinRel(sm, 1e-4f));
    }
  }
}
