#include "specula/util/spectrum/rgb_illuminant_spectrum.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/color.hpp"
#include "specula/util/colorspace.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/spectrum.hpp"
#include "specula/util/spectrum/functions.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("RgbIlluminantSpectrum", "[util][spectrum]") {
  SECTION("Round trip sRGB") {
    Rng rng;
    const RgbColorSpace &cs = *RgbColorSpace::SRGB;

    for (size_t i = 0; i < 100; ++i) {
      Rgb rgb(rng.uniform<Float>(), rng.uniform<Float>(), rng.uniform<Float>());
      RgbIlluminantSpectrum rs(cs, rgb);

      DenselySampledSpectrum rs_illum =
          DenselySampledSpectrum::sample_function([&](Float lambda) { return rs(lambda); });
      Xyz xyz = spectrum_to_xyz(&rs_illum);
      Rgb out = cs.to_rgb(xyz);

      CHECK_THAT(out.r, WithinAbs(rgb.r, 0.01));
      CHECK_THAT(out.g, WithinAbs(rgb.g, 0.01));
      CHECK_THAT(out.b, WithinAbs(rgb.b, 0.01));
    }
  }

  SECTION("Round trip Rec2020") {
    Rng rng;
    const RgbColorSpace &cs = *RgbColorSpace::REC_2020;

    for (size_t i = 0; i < 100; ++i) {
      Rgb rgb(0.1 + 0.7 * rng.uniform<Float>(), 0.1 + 0.7 * rng.uniform<Float>(),
              0.1 + 0.7 * rng.uniform<Float>());
      RgbIlluminantSpectrum rs(cs, rgb);

      DenselySampledSpectrum rs_illum =
          DenselySampledSpectrum::sample_function([&](Float lambda) { return rs(lambda); });
      Xyz xyz = spectrum_to_xyz(&rs_illum);
      Rgb out = cs.to_rgb(xyz);

      CHECK_THAT(out.r, WithinAbs(rgb.r, 0.01));
      CHECK_THAT(out.g, WithinAbs(rgb.g, 0.01));
      CHECK_THAT(out.b, WithinAbs(rgb.b, 0.01));
    }
  }

  SECTION("Round trip ACES") {
    Rng rng;
    const RgbColorSpace &cs = *RgbColorSpace::ACES2065_1;

    for (size_t i = 0; i < 100; ++i) {
      Rgb rgb(0.3 + 0.4 * rng.uniform<Float>(), 0.3 + 0.4 * rng.uniform<Float>(),
              0.3 + 0.4 * rng.uniform<Float>());
      RgbIlluminantSpectrum rs(cs, rgb);

      DenselySampledSpectrum rs_illum =
          DenselySampledSpectrum::sample_function([&](Float lambda) { return rs(lambda); });
      Xyz xyz = spectrum_to_xyz(&rs_illum);
      Rgb out = cs.to_rgb(xyz);

      CHECK_THAT(out.r, WithinAbs(rgb.r, 0.01));
      CHECK_THAT(out.g, WithinAbs(rgb.g, 0.01));
      CHECK_THAT(out.b, WithinAbs(rgb.b, 0.01));
    }
  }
}
