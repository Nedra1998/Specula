#include "specula/util/splines.hpp"

#include <catch2/catch_test_macros.hpp>
#include <specula/util/pstd.hpp>
#include <specula/util/rng.hpp>
#include <specula/util/vecmath.hpp>

using namespace specula;

TEST_CASE("Spline", "[util]") {
  SECTION("Bezier bounds") {
    std::array<Point3f, 4> cp;

    Bounds3f b = bound_cubic_bezier(pstd::make_const_span(cp), 0.0f, 1.0f);
    b = expand(b, 1e-3 * length(b.diagonal()));
    for (Float u = 0.0f; u <= 1.0f; u += 1.0f / 1024.0f) {
      Point3f p = evaluate_cubic_bezier(pstd::make_const_span(cp), u);
      CHECK(inside(p, b));
    }

    Rng rng;
    for (int i = 0; i < 1'000; ++i) {
      for (int j = 0; j < 4; ++j) {
        for (int c = 0; c < 3; ++c) {
          cp[j][c] = -5.0f + 10.0f * rng.uniform<Float>();
        }
      }

      Bounds3f b = bound_cubic_bezier(pstd::make_const_span(cp), 0.0f, 1.0f);
      b = expand(b, 1e-3 * length(b.diagonal()));
      for (Float u = 0.0f; u <= 1.0f; u += 1.0f / 1024.0f) {
        Point3f p = evaluate_cubic_bezier(pstd::make_const_span(cp), u);
        CHECK(inside(p, b));
      }
    }
  }
}
