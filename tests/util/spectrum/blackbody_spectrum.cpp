#include "specula/util/spectrum/blackbody_spectrum.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/macros.hpp"
#include "specula/util/spectrum.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("BlackBodySpectrum", "[util][spectrum]") {
  Float v[][3] = {
      {483, 6000, 3.1849e13},
      {600, 6000, 2.86772e13},
      {500, 3700, 1.59845e12},
      {600, 4500, 7.46497e12},
  };

  int n = SPECULA_ARRAYSIZE(v);
  for (int i = 0; i < n; ++i) {
    Float lambda = v[i][0], t = v[i][1], le_expected = v[i][2];
    CHECK_THAT(blackbody(lambda, t), WithinRel(le_expected, Float(0.001)));
  }

  for (Float t : {2700, 3000, 4500, 5600, 6000}) {
    Float lambda_max = 2.8977721e-3 / t * 1e9;
    CHECK(blackbody(0.99 * lambda_max, t) < blackbody(lambda_max, t));
    CHECK(blackbody(lambda_max, t) > blackbody(1.01 * lambda_max, t));
  }
}
