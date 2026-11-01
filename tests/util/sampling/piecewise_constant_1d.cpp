#include "specula/util/sampling/piecewise_constant_1d.hpp"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/sampling.hpp"
#include "specula/util/sampling/functions.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("PiecewiseConstant1D", "[util][sampling]") {
  SECTION("Continuous") {
    PiecewiseConstant1D dist({1.0f, 1.0f, 2.0f, 4.0f, 8.0f});
    CHECK(dist.size() == 5);

    Float pdf = NAN;
    int offset = 0;
    CHECK(dist.sample(0.0, &pdf, &offset) == 0.0);
    CHECK(offset == 0);

    CHECK_THAT(dist.sample(0.5, &pdf, &offset), WithinAbs(0.8, 1e-6));

    CHECK_THAT(dist.sample(0.75, &pdf, &offset), WithinAbs(0.9, 1e-6));
    CHECK_THAT(pdf, WithinRel(dist.size() * 8.0 / 16.0));
    CHECK(offset == 4);

    CHECK_THAT(dist.sample(0.0, &pdf), WithinRel(0.0));
    CHECK_THAT(dist.sample(1.0, &pdf), WithinRel(1.0));
  }

  SECTION("Range") {
    auto values = sample_1d_function([](Float x) { return 1 + x; }, 65536, 4, -1.0f, 3.0f);
    PiecewiseConstant1D dist(values, -1.0f, 3.0f);

    for (Float u : Uniform1D(100)) {
      Float pd = NAN;
      Float xd = dist.sample(u, &pd);

      Float t0 = NAN, t1 = NAN;
      REQUIRE(quadratic(1.0 / 16.0, 1.0 / 8.0, 1.0 / 16.0 - u, &t0, &t1));
      Float xa = (t0 >= -1 && t0 <= 3) ? t0 : t1;
      Float pa = (1 + xa) / 8.0;

      CHECK_THAT(xd, WithinAbs(xa, 2e-3));
      CHECK_THAT(pd, WithinAbs(pa, 2e-3));
    }
  }

  SECTION("Inverse uniform") {
    std::vector<Float> values = {1.0, 1.0, 1.0};

    PiecewiseConstant1D dist(values);
    CHECK(*dist.invert(0) == 0);
    CHECK(*dist.invert(0.5) == 0.5);
    CHECK(*dist.invert(0.75) == 0.75);

    PiecewiseConstant1D dist2(values, -1, 3);
    CHECK(*dist2.invert(-1) == 0);
    CHECK(*dist2.invert(0) == 0.25);
    CHECK(*dist2.invert(1) == 0.5);
    CHECK(*dist2.invert(2) == 0.75);
    CHECK(*dist2.invert(3) == 1);
  }

  SECTION("Inverse general") {
    std::vector<Float> values = {0, 0.25, 0.5, 0.25};

    PiecewiseConstant1D dist(values);
    CHECK(*dist.invert(0) == 0);
    CHECK(*dist.invert(1) == 1);
    CHECK(*dist.invert(0.5) == 0.25);
    CHECK(*dist.invert(0.625) == 0.5);
    CHECK(*dist.invert(0.75) == 0.75);
    CHECK_THAT(*dist.invert(0.825), WithinAbs(0.825, 1e-6));

    PiecewiseConstant1D dist2(values, -1, 3);
    CHECK(*dist2.invert(-1) == 0);
    CHECK(*dist2.invert(3) == 1);
    CHECK(*dist2.invert(lerp(0.5, -1, 3)) == 0.25);
    CHECK(*dist2.invert(lerp(0.625, -1, 3)) == 0.5);
    CHECK(*dist2.invert(lerp(0.75, -1, 3)) == 0.75);
    CHECK_THAT(*dist2.invert(lerp(0.825, -1, 3)), WithinAbs(0.825, 1e-6));
  }

  SECTION("Inverse randoms") {
    std::vector<Float> values = {0, 1.25, 0.5, 0.25, 3.7};

    PiecewiseConstant1D dist(values);
    for (Float u : Uniform1D(100)) {
      Float v = dist.sample(u);
      auto inv = dist.invert(v);
      REQUIRE(inv.has_value());
      CHECK_THAT(*inv, WithinRel(u, Float(1e-4)));
    }

    PiecewiseConstant1D dist2(values, -3, 3);
    for (Float u : Uniform1D(100)) {
      Float v = dist.sample(u);
      auto inv = dist.invert(v);
      REQUIRE(inv.has_value());
      CHECK_THAT(*inv, WithinRel(u, Float(1e-4)));
    }
  }

  SECTION("Integral") {
    std::vector<Float> values{1.0, 0.5, 0.0, 0.25};

    PiecewiseConstant1D dist(values);
    CHECK(dist.integral() == 0.4375);

    PiecewiseConstant1D dist2(values, 0, 4);
    CHECK(dist2.integral() == 1.75);

    PiecewiseConstant1D dist3(values, -3, 5);
    CHECK(dist3.integral() == 3.5);
  }
}
