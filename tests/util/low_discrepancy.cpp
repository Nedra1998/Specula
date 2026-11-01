#include "specula/util/low_discrepancy.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/math.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("LowDiscrepancy", "[util]") {
  SECTION("Radical inverse") {
    for (int a = 0; a < 1024; ++a) {
      CHECK(radical_inverse(0, a) == reverse_bits_32(a) * 2.3283064365386963e-10f);
    }
  }

  SECTION("Sobol first dimension") {
    for (int i = 0; i < 8192; ++i) {
      CHECK(sobol_sample(i, 0, NoRandomizer{}) == reverse_bits_32(i) * 2.3283064365386963e-10f);
    }
  }
}

TEST_CASE("Sobol", "[util]") {
  SECTION("Interval to index") {
    for (int log_res = 0; log_res < 8; ++log_res) {
      int res = 1 << log_res;
      for (int y = 0; y < res; ++y) {
        for (int x = 0; x < res; ++x) {
          bool saw_corner = false;
          for (int s = 0; s < 16; ++s) {
            uint64_t seq_index = sobol_interval_to_index(log_res, s, {x, y});
            Point2f samp(sobol_sample(seq_index, 0, NoRandomizer{}),
                         sobol_sample(seq_index, 1, NoRandomizer{}));
            Point2f ss(samp[0] * res, samp[1] * res);

            if (ss == Point2f(x, y)) {
              CHECK_FALSE(saw_corner);
              saw_corner = true;
            }

            CAPTURE(Point2i(x, y), Point2i(ss));
            bool matches = Point2i(x, y) == Point2i(ss) ||
                           (x == static_cast<int>(ss.x) && static_cast<Float>(y + 1) == ss.y) ||
                           (static_cast<Float>(x + 1) == ss.x && y == static_cast<int>(ss.y));
            CHECK(matches);
          }
        }
      }
    }
  }

  SECTION("Interval to index randoms") {
    Rng rng;
    for (int i = 0; i < 100'000; ++i) {
      int log_res = rng.uniform<int>(16);
      int res = 1 << log_res;
      int x = rng.uniform<int>(res), y = rng.uniform<int>(res);
      int s = rng.uniform<int>(8192);

      uint64_t seq_index = sobol_interval_to_index(log_res, s, {x, y});
      Point2f samp(sobol_sample(seq_index, 0, NoRandomizer{}),
                   sobol_sample(seq_index, 1, NoRandomizer{}));
      Point2f ss(static_cast<int>(samp[0] * res), static_cast<int>(samp[1] * res));

      CAPTURE(Point2i(x, y), s, ss);
      bool matches = Point2i(x, y) == Point2i(ss) ||
                     (x == static_cast<int>(ss.x) && static_cast<Float>(y + 1) == ss.y) ||
                     (static_cast<Float>(x + 1) == ss.x && y == static_cast<int>(ss.y));
      CHECK(matches);
    }
  }
}
