#include "specula/util/sampling/summed_area_table.hpp"

#include <numeric>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <spdlog/common.h>

#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("SummedAreaTable", "[util][sampling]") {
  SECTION("Constant") {
    Array2D<Float> v(4, 4);

    for (int y = 0; y < v.ysize(); ++y) {
      for (int x = 0; x < v.xsize(); ++x) {
        v(x, y) = 1;
      }
    }

    SummedAreaTable sat(v);

    CHECK(sat.integral(Bounds2f({0, 0}, {1, 1})) == 1);
    CHECK(sat.integral(Bounds2f({0, 0}, {1, 0.5})) == 0.5);
    CHECK(sat.integral(Bounds2f({0, 0}, {0.5, 1})) == 0.5);
    CHECK(sat.integral(Bounds2f({0, 0}, {0.25, 0.75})) == 3.0 / 16.0);
    CHECK(sat.integral(Bounds2f({0.5, 0.25}, {0.75, 1})) == 3.0 / 16.0);
  }

  SECTION("Rect") {
    Array2D<Float> v(8, 4);

    for (int y = 0; y < v.ysize(); ++y) {
      for (int x = 0; x < v.xsize(); ++x) {
        v(x, y) = x + y;
      }
    }

    SummedAreaTable sat(v);

    for (int y0 = 0; y0 < v.ysize(); ++y0) {
      for (int x0 = 0; x0 < v.xsize(); ++x0) {
        for (int y1 = y0; y1 < v.ysize(); ++y1) {
          for (int x1 = x0; x1 < v.xsize(); ++x1) {
            Float sum = 0;
            for (int y = y0; y < y1; ++y) {
              for (int x = x0; x < x1; ++x) {
                sum += v(x, y);
              }
            }

            Bounds2f b({static_cast<Float>(x0) / v.xsize(), static_cast<Float>(y0) / v.ysize()},
                       {static_cast<Float>(x1) / v.xsize(), static_cast<Float>(y1) / v.ysize()});
            CHECK(sat.integral(b) == sum / (v.xsize() * v.ysize()));
          }
        }
      }
    }
  }

  SECTION("Randoms") {
    std::array<int, 2> dims[] = {{1, 6}, {6, 1}, {12, 19}, {16, 16}, {100, 300}, {49, 2}};
    Rng rng;

    for (const auto d : dims) {
      Array2D<Float> v(d[0], d[1]);
      for (int y = 0; y < v.ysize(); ++y) {
        for (int x = 0; x < v.xsize(); ++x) {
          v(x, y) = rng.uniform<int>(32);
        }
      }

      SummedAreaTable sat(v);

      for (int i = 0; i < 100; ++i) {
        Bounds2i bi({rng.uniform<int>(v.xsize()), rng.uniform<int>(v.ysize())},
                    {rng.uniform<int>(v.xsize()), rng.uniform<int>(v.ysize())});
        Bounds2f bf({static_cast<Float>(bi.p_min.x) / static_cast<Float>(v.xsize()),
                     static_cast<Float>(bi.p_min.y) / static_cast<Float>(v.ysize())},
                    {static_cast<Float>(bi.p_max.x) / static_cast<Float>(v.xsize()),
                     static_cast<Float>(bi.p_max.y) / static_cast<Float>(v.ysize())});
        double ref = 0;
        for (Point2i p : bi) {
          ref += v[p];
        }
        ref /= v.xsize() * v.ysize();

        CHECK_THAT(sat.integral(bf), WithinRel(ref, 1e-3));
      }
    }
  }

  SECTION("non-cell aligned") {
    std::array<int, 2> dims[] = {{1, 6}, {6, 1}, {12, 19}, {16, 16}, {100, 300}, {49, 2}};
    Rng rng;

    for (const auto d : dims) {
      Array2D<Float> v(d[0], d[1]);
      for (int y = 0; y < v.ysize(); ++y) {
        for (int x = 0; x < v.xsize(); ++x) {
          v(x, y) = rng.uniform<int>(32);
        }
      }

      SummedAreaTable sat(v);
      Bounds2f b({rng.uniform<Float>(), rng.uniform<Float>()},
                 {rng.uniform<Float>(), rng.uniform<Float>()});

      Float sampled_sum = 0;
      int nsamples = 100'000;
      for (Point2f u : Hammersley2D(nsamples)) {
        Point2f p = b.lerp(u);
        Point2i pi(p.x * v.xsize(), p.y * v.ysize());
        sampled_sum += v[pi];
      }
      Float sampled_result = sampled_sum * b.area() / nsamples;

      CHECK_THAT(sat.integral(b), WithinAbs(sampled_result, 1e-3f));
    }
  }
}
