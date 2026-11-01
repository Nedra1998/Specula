#include "specula/util/sampling/piecewise_constant_2d.hpp"

#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

namespace specula {
  void compare_distributions(const PiecewiseConstant1D &da, const PiecewiseConstant1D &db,
                             Float eps = 1e-5) {
    REQUIRE(da.func.size() == db.func.size());
    REQUIRE(da.cdf.size() == db.cdf.size());
    REQUIRE(da.min == db.min);
    REQUIRE(da.max == db.max);

    for (size_t i = 0; i < da.func.size(); ++i) {
      Float pdfa = da.func[i] / da.func_int, pdfb = db.func[i] / db.func_int;
      CHECK_THAT(pdfa, WithinAbs(pdfb, eps));
    }
  }

  void compare_distributions(const PiecewiseConstant2D &da, const PiecewiseConstant2D &db,
                             Float eps = 1e-5) {
    compare_distributions(da.pmarginal, db.pmarginal, eps);
    REQUIRE(da.pconditionalv.size() == db.pconditionalv.size());
    REQUIRE(da.domain_ == db.domain_);
    for (size_t i = 0; i < da.pconditionalv.size(); ++i) {
      compare_distributions(da.pconditionalv[i], db.pconditionalv[i], eps);
    }
  }
} // namespace specula

TEST_CASE("PiecewiseConstant2D", "[util][sampling]") {
  SECTION("Inverse uniform") {
    std::vector<Float> values = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

    PiecewiseConstant2D dist(values, 3, 2);
    CHECK(*dist.invert({0, 0}) == Point2f(0, 0));
    CHECK(*dist.invert({1, 1}) == Point2f(1, 1));
    CHECK(*dist.invert({0.5, 0.5}) == Point2f(0.5, 0.5));
    CHECK(*dist.invert({0.25, 0.75}) == Point2f(0.25, 0.75));

    Bounds2f domain(Point2f(-1, -0.5), Point2f(3, 1.5));
    PiecewiseConstant2D dist2(values, 3, 2, domain);
    CHECK(*dist2.invert(domain.lerp({0, 0})) == Point2f(0, 0));
    CHECK(*dist2.invert(domain.lerp({1, 1})) == Point2f(1, 1));
    CHECK(*dist2.invert(domain.lerp({0.5, 0.5})) == Point2f(0.5, 0.5));
    CHECK(*dist2.invert(domain.lerp({0.25, 0.75})) == Point2f(0.25, 0.75));
  }

  SECTION("Inverse randoms") {
    int nx = 4, ny = 5;
    std::vector<Float> values(static_cast<size_t>(nx * ny));
    Rng rng;
    for (int i = 0; i < nx * ny; ++i) {
      values[i] = rng.uniform<Float>();
    }

    PiecewiseConstant2D dist(values, nx, ny);
    for (Point2f u : Uniform2D(100)) {
      Point2f v = dist.sample(u);
      auto inv = dist.invert(v);
      REQUIRE(inv.has_value());
      CHECK_THAT(inv->x, WithinRel(u.x, Float(1e-4)));
      CHECK_THAT(inv->y, WithinRel(u.y, Float(1e-4)));
    }

    Bounds2f domain(Point2f(-1, -0.5), Point2f(3, 1.5));
    PiecewiseConstant2D dist2(values, nx, ny, domain);
    for (Point2f u : Uniform2D(100)) {
      Point2f v = dist2.sample(u);
      auto inv = dist2.invert(v);
      REQUIRE(inv.has_value());
      CHECK_THAT(inv->x, WithinRel(u.x, Float(1e-4)));
      CHECK_THAT(inv->y, WithinRel(u.y, Float(1e-4)));
    }
  }

  SECTION("From func infinity") {
    auto f = [](Float x, Float y) { return x * x * y; };
    auto values = sample_2d_function(f, 4, 2, 1, Bounds2f(Point2f(0, 0), Point2f(1, 1)));
    PiecewiseConstant2D dsampled(values, 4, 2);

    std::vector<Float> exact = {
        sqr(0.25) * 0.5, sqr(0.5) * 0.5, sqr(0.75) * 0.5, sqr(1) * 0.5,
        sqr(0.25) * 1.0, sqr(0.5) * 1.0, sqr(0.75) * 1.0, sqr(1) * 1.0,
    };
    PiecewiseConstant2D dexact(exact, 4, 2);
    compare_distributions(dsampled, dexact);
  }

  SECTION("Integral") {
    std::vector<Float> values{1.0, 2, 0, 7, 3, 1, 2, 0};

    PiecewiseConstant2D dist(values, 4, 2);
    CHECK(dist.integral() == 2);

    PiecewiseConstant2D dist2(values, 2, 4);
    CHECK(dist2.integral() == 2);

    PiecewiseConstant2D dist3(values, 4, 2, Bounds2f({0, 0}, {4, 2}));
    CHECK(dist3.integral() == 16);

    PiecewiseConstant2D dist4(values, 4, 2, Bounds2f({-1, 0}, {0, 4}));
    CHECK(dist4.integral() == 8);
  }
}
