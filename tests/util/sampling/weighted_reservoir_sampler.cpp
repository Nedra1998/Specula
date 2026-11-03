#include "specula/util/sampling/weighted_reservoir_sampler.hpp"

#include <numeric>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <spdlog/common.h>

#include "specula/util/parallel.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("WeightedReservoir", "[util][sampling]") {
  SECTION("Basic") {
    Rng rng;
    constexpr int n = 16;
    float weights[n];
    for (float &weight : weights) {
      weight = 0.05 * sqr(rng.uniform<Float>());
    }
    std::atomic<int> count[n] = {};
    int64_t ntrials = 1'000'000;
    parallel_for(0, ntrials, [&](int64_t start, int64_t end) {
      Rng rng(3 * start);
      int local_count[n] = {};

      for (int64_t i = start; i < end; ++i) {
        WeightedReservoirSampler<int> wrs(i);
        int perm[n];
        for (int j = 0; j < n; ++j) {
          perm[j] = j;
        }
        for (int index : perm) {
          wrs.add(index, weights[index]);
        }

        int index = wrs.get_sample();
        REQUIRE(index >= 0);
        REQUIRE(index < n);
        ++local_count[index];
      }

      for (int i = 0; i < n; ++i) {
        count[i] += local_count[i];
      }
    });

    Float sumw = std::accumulate(std::begin(weights), std::end(weights), static_cast<Float>(0));
    for (int i = 0; i < n; ++i) {
      CHECK_THAT(weights[i] / sumw, WithinAbs(count[i] / double(ntrials), 0.03));
    }
  }

  SECTION("Merge reservoirs") {
    Rng rng(6502);
    constexpr int n = 8;
    float weights[n];
    for (float &weight : weights) {
      weight = 0.01 + rng.uniform<Float>();
    }

    std::atomic<int> count[] = {};
    int64_t ntrials = 1'000'000;
    parallel_for(0, ntrials, [&](int64_t start, int64_t end) {
      int local_count[n] = {};

      for (int64_t i = start; i < end; ++i) {
        WeightedReservoirSampler<int> wrs0(i);
        WeightedReservoirSampler<int> wrs1(i + 1);

        for (int j = 0; j < n; ++j) {
          if (j & 1) {
            wrs0.add(j, weights[j]);
          } else {
            wrs1.add(j, weights[j]);
          }
        }

        wrs0.merge(wrs1);
        ++local_count[wrs0.get_sample()];
      }

      for (int i = 0; i < n; ++i) {
        count[i] += local_count[i];
      }
    });

    Float sumw = std::accumulate(std::begin(weights), std::end(weights), static_cast<Float>(0));
    for (int i = 0; i < n; ++i) {
      CHECK_THAT(weights[i] / sumw, WithinAbs(count[i] / double(ntrials), 0.02));
    }
  }
}
