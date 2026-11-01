#include "specula/util/spectrum/sampled_spectrum.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/color.hpp"
#include "specula/util/sampling.hpp"
#include "specula/util/spectrum.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("SampledSpectrum", "[util][spectrum]") {
  SECTION("XYZ of constant spectrum is one") {
    std::array<Float, 3> xyz_sum = {0};
    int n = 100;
    for (Float u : Stratified1D(n)) {
      SampledWavelengths lambda = SampledWavelengths::sample_uniform(u, LAMBDA_MIN, LAMBDA_MAX);
      Xyz xyz = SampledSpectrum(1.0).to_xyz(lambda);
      for (int c = 0; c < 3; ++c) {
        xyz_sum[c] += xyz[c];
      }
    }

    for (int c = 0; c < 3; ++c) {
      xyz_sum[c] /= n;
    }

    CHECK_THAT(xyz_sum[0], WithinAbs(1.0, 0.035));
    CHECK_THAT(xyz_sum[1], WithinAbs(1.0, 0.035));
    CHECK_THAT(xyz_sum[2], WithinAbs(1.0, 0.035));
  }
}
