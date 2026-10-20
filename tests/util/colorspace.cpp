#include "specula/util/colorspace.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/color.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("RgbColorSpace", "[util]") {
  SECTION("Rgb to Xyz roundtrip") {
    const auto cs =
        GENERATE(*RgbColorSpace::ACES2065_1, *RgbColorSpace::REC_2020, *RgbColorSpace::SRGB);
    Xyz xyz = cs.to_xyz({1, 1, 1});
    Rgb rgb = cs.to_rgb(xyz);
    CHECK_THAT(rgb[0], WithinAbs(1.0, 1e-4));
    CHECK_THAT(rgb[1], WithinAbs(1.0, 1e-4));
    CHECK_THAT(rgb[2], WithinAbs(1.0, 1e-4));
  }

  SECTION("sRGB") {
    const RgbColorSpace &srgb = *RgbColorSpace::SRGB;

    Rgb rgb = srgb.to_rgb({1, 0, 0});
    CHECK_THAT(rgb[0], WithinAbs(3.2406, 1e-3));
    CHECK_THAT(rgb[1], WithinAbs(-0.9689, 1e-3));
    CHECK_THAT(rgb[2], WithinAbs(0.0557, 1e-3));

    rgb = srgb.to_rgb({0, 1, 0});
    CHECK_THAT(rgb[0], WithinAbs(-1.5372, 1e-3));
    CHECK_THAT(rgb[1], WithinAbs(1.8758, 1e-3));
    CHECK_THAT(rgb[2], WithinAbs(-0.2040, 1e-3));

    rgb = srgb.to_rgb({0, 0, 1});
    CHECK_THAT(rgb[0], WithinAbs(-0.4986, 1e-3));
    CHECK_THAT(rgb[1], WithinAbs(0.0415, 1e-3));
    CHECK_THAT(rgb[2], WithinAbs(1.0570, 1e-3));
  }

  SECTION("Illuminant sRGB") {
    Xyz xyz = spectrum_to_xyz(&RgbColorSpace::SRGB->illuminant);
    Rgb rgb = RgbColorSpace::SRGB->to_rgb(xyz);
    CHECK_THAT(rgb.r, WithinAbs(1.0, 0.01));
    CHECK_THAT(rgb.g, WithinAbs(1.0, 0.01));
    CHECK_THAT(rgb.b, WithinAbs(1.0, 0.01));
  }

  SECTION("Illuminant Rec2020") {
    Xyz xyz = spectrum_to_xyz(&RgbColorSpace::REC_2020->illuminant);
    Rgb rgb = RgbColorSpace::REC_2020->to_rgb(xyz);
    CHECK_THAT(rgb.r, WithinAbs(1.0, 0.01));
    CHECK_THAT(rgb.g, WithinAbs(1.0, 0.01));
    CHECK_THAT(rgb.b, WithinAbs(1.0, 0.01));
  }

  SECTION("Illuminant ACES2065_1") {
    Xyz xyz = spectrum_to_xyz(&RgbColorSpace::ACES2065_1->illuminant);
    Rgb rgb = RgbColorSpace::ACES2065_1->to_rgb(xyz);
    CHECK_THAT(rgb.r, WithinAbs(1.0, 0.01));
    CHECK_THAT(rgb.g, WithinAbs(1.0, 0.01));
    CHECK_THAT(rgb.b, WithinAbs(1.0, 0.01));
  }
}
