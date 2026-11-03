#include "specula/util/sampling/alias_table.hpp"

#include <numeric>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <spdlog/common.h>

#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("AliasTable", "[util][sampling]") {
  SECTION("Uniform") {
    Float values[5] = {2.0f, 2.0f, 2.0f, 2.0f, 2.0f};
    AliasTable table(values);
    CHECK(table.size() == 5);

    int count[5] = {0};
    int iters = 10'000;
    for (Float u : Stratified1D(iters)) {
      Float pdf = NAN;
      int offset = table.sample(u, &pdf);
      REQUIRE(offset >= 0);
      REQUIRE(offset < 5);
      ++count[offset];
      CHECK_THAT(pdf, WithinRel(1.0 / 5.0, 0.01));
    }

    for (int i : count) {
      CHECK_THAT(Float(i) / iters, WithinRel(1.0 / 5.0, 0.01));
    }
  }

  SECTION("Varying") {
    Rng rng;
    std::vector<Float> values;
    int n = 103;
    values.reserve(n);
    for (int i = 0; i < n; ++i) {
      values.push_back(0.25f * 2 * rng.uniform<Float>());
    }
    Float sum = std::accumulate(values.begin(), values.end(), static_cast<Float>(0));

    AliasTable table(values);
    CHECK(values.size() == n);

    std::vector<int> count(n, 0);
    int iters = 1'000'000;
    for (Float u : Stratified1D(iters)) {
      Float pdf = NAN;
      int offset = table.sample(u, &pdf);
      REQUIRE(offset >= 0);
      REQUIRE(offset < n);
      ++count[offset];

      Float ref_pdf = values[offset] / sum;
      CHECK_THAT(pdf, WithinRel(ref_pdf, Float(0.01)));
    }

    for (int i = 0; i < n; ++i) {
      Float pdf = values[i] / sum;
      CHECK_THAT(Float(count[i]) / iters, WithinRel(pdf, Float(0.01)));
    }
  }
}
