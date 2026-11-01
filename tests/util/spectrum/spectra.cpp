#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/spectrum.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Spectra", "[util][spectrum]") {
  SECTION("Integral of spectra is one in XYZ") {
    Float xx = 0, yy = 0, zz = 0;
    for (Float lambda = LAMBDA_MIN; lambda < LAMBDA_MAX; ++lambda) {
      xx += spectra::X()(lambda);
      yy += spectra::Y()(lambda);
      zz += spectra::Z()(lambda);
    }
    xx /= CIE_Y_INTEGRAL;
    yy /= CIE_Y_INTEGRAL;
    zz /= CIE_Y_INTEGRAL;

    CHECK_THAT(xx, WithinAbs(1.0, 0.005));
    CHECK_THAT(yy, WithinAbs(1.0, 0.005));
    CHECK_THAT(zz, WithinAbs(1.0, 0.005));
  }
}
