#include "specula/util/sampling/variance_estimator.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("VarianceEstimator", "[util][sampling]") {
  SECTION("zero") {
    VarianceEstimator<Float> ve;
    for (int i = 0; i < 100; ++i) {
      ve.add(10.0);
    }
    CHECK(ve.variance() == 0);
  }

  SECTION("vs closed form") {
    VarianceEstimator<double> ve;
    int count = 10'000;
    double sum = 0;
    for (Float u : Stratified1D(count)) {
      Float v = lerp(u, -1, 1);
      ve.add(v);
      sum += v;
    }

    CHECK_THAT(ve.variance(), WithinAbs(1.0 / 3.0, 1e-3));
    CHECK_THAT(ve.mean(), WithinAbs(sum / count, 1e-5));
  }

  SECTION("merge") {
    int n = 16;
    std::vector<VarianceEstimator<double>> ve(n);

    Rng rng;
    int count = 10'000;
    double sum = 0;
    for (Float u : Stratified1D(count)) {
      Float v = lerp(u, -1, 1);
      int index = rng.uniform<int>(ve.size());
      ve[index].add(v);
      sum += v;
    }

    VarianceEstimator<double> ve_final;
    for (const auto &v : ve) {
      ve_final.merge(v);
    }

    CHECK_THAT(ve_final.variance(), WithinAbs(1.0 / 3.0, 1e-3));
    CHECK_THAT(ve_final.mean(), WithinAbs(sum / count, 1e-5));
  }

  SECTION("merge two") {
    VarianceEstimator<double> ve[2], veboth;

    int index = 0;
    for (Float u : Stratified1D(1000)) {
      if (index++ < 400) {
        ve[0].add(u * u * u);
      } else {
        ve[1].add(u * u * u);
      }
      veboth.add(u * u * u);
    }

    ve[0].merge(ve[1]);

    CHECK_THAT(ve[0].mean(), WithinRel(veboth.mean(), 1e-5));
    CHECK_THAT(ve[0].variance(), WithinRel(veboth.variance(), 1e-5));
  }
}
