#include "specula/util/vecmath/tuple3.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/math.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Vector3", "[util][vecmath]") {
  SECTION("Basics") {
    Vector3f vf(-1, 10, 2);

    CHECK(vf == Vector3f(Vector3i(-1, 10, 2)));
    CHECK(vf != Vector3f(-1, 100, 2));

    CHECK(vf + vf == Vector3f(-2, 20, 4));
    CHECK(vf - vf == Vector3f(0, 0, 0));
    CHECK(vf * 2 == Vector3f(-2, 20, 4));
    CHECK(2 * vf == Vector3f(-2, 20, 4));
    CHECK(vf / 2 == Vector3f(-0.5, 5, 1));

    CHECK(abs(vf) == Vector3f(1, 10, 2));

    CHECK(vf == ceil(Vector3f(-1.5, 9.9, 1.01)));
    CHECK(vf == floor(Vector3f(-0.5, 10.01, 2.99)));

    CHECK(min(vf, Vector3f(-20, 20, 1.5)) == Vector3f(-20, 10, 1.5));
    CHECK(max(vf, Vector3f(-20, 20, 0)) == Vector3f(-1, 20, 2));

    CHECK(min_component_value(vf) == -1);
    CHECK(min_component_value(-vf) == -10);
    CHECK(max_component_value(vf) == 10);
    CHECK(max_component_value(-vf) == 1);

    CHECK(min_component_index(vf) == 0);
    CHECK(min_component_index(-vf) == 1);
    CHECK(max_component_index(vf) == 1);
    CHECK(max_component_index(-vf) == 0);

    CHECK(permute(vf, {0, 1, 2}) == vf);
    CHECK(permute(vf, {1, 0, 2}) == Vector3f(10, -1, 2));
    CHECK(permute(vf, {2, 0, 1}) == Vector3f(2, -1, 10));
    CHECK(permute(vf, {1, 1, 0}) == Vector3f(10, 10, -1));
  }

  SECTION("Angle between") {
    CHECK(angle_between(Vector3f(1, 0, 0), Vector3f(1, 0, 0)) == 0);

    CHECK_THAT(angle_between(Vector3f(0, 0, 1), Vector3f(0, 0, -1)),
               WithinAbs(std::numbers::pi, 1e-7));
    CHECK_THAT(angle_between(Vector3f(1, 0, 0), Vector3f(0, 1, 0)),
               WithinAbs(std::numbers::pi / 2.0, 1e-7));

    Vector3f x = normalize(Vector3f(1, -3, 10));
    CHECK(angle_between(x, x) == 0);
    CHECK_THAT(angle_between(x, -x), WithinAbs(PI, 3e-7));

    Float max_err = 0, sum_err = 0;
    for (int i = 0; i < 100'000; ++i) {
      Rng rng(i);

      Vector3f a = normalize(Vector3f(-1 + 2 * rng.uniform<Float>(), -1 + 2 * rng.uniform<Float>(),
                                      -1 + 2 * rng.uniform<Float>()));
      Vector3f b = normalize(Vector3f(-1 + 2 * rng.uniform<Float>(), -1 + 2 * rng.uniform<Float>(),
                                      -1 + 2 * rng.uniform<Float>()));

      Vector3<double> ad(a), bd(b);
      ad = normalize(ad);
      bd = normalize(bd);

      Float v[2] = {Float(std::acos(dot(ad, bd))), angle_between(a, b)};
      Float err = std::abs(v[0] - v[1]) / v[0];
      max_err = std::max(err, max_err);
      sum_err += err;
      CHECK(err < 5e-6);
    }

    max_err = sum_err = 0;
    for (int i = 0; i < 100'000; ++i) {
      Rng rng(i + 10'000'000);

      Vector3f a = normalize(Vector3f(-1 + 2 * rng.uniform<Float>(), -1 + 2 * rng.uniform<Float>(),
                                      -1 + 2 * rng.uniform<Float>()));
      Vector3f b = normalize(Vector3f(-1 + 2 * rng.uniform<Float>(), -1 + 2 * rng.uniform<Float>(),
                                      -1 + 2 * rng.uniform<Float>()));

      a.x = std::copysign(a.x, -b.x);
      a.y = std::copysign(a.y, -b.y);
      a.z = std::copysign(a.z, -b.z);

      Vector3<double> ad(a), bd(b);
      ad = normalize(ad);
      bd = normalize(bd);

      Float v[2] = {Float(std::acos(dot(ad, bd))), angle_between(a, b)};
      Float err = std::abs(v[0] - v[1]) / v[0];
      max_err = std::max(err, max_err);
      sum_err += err;
      CHECK(err < 5e-6);
    }

    Vector3f a(1, 1, 1), b(-1, -1.0001, -1);
    a = normalize(a);
    b = normalize(b);
    Vector3<long double> ad(1, 1, 1), bd(-1, -1.0001, -1);
    ad = normalize(ad);
    bd = normalize(bd);

    Float naive = safe_acos(dot(a, b));
    Float precise = std::acos(clamp(dot(ad, bd), -1, 1));
    Float abet = angle_between(a, b);
    CHECK(abet == precise);
  }

  SECTION("Coordinate system") {
    auto error = [](Vector3f a, Vector3f b, Vector3f c) {
      return (sqr(length(a) - 1) + sqr(length(b) - 1) + sqr(length(c) - 1) + sqr(dot(a, b)) +
              sqr(dot(b, c)) + sqr(dot(c, a))) /
             6;
    };

    Vector3f a, b;
    for (Vector3f v : {Vector3f(1, 0, 0), Vector3f(-1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, -1, 0),
                       Vector3f(0, 0, 1), Vector3f(0, 0, -1)}) {
      coordinate_system(v, &a, &b);
      for (int c = 0; c < 3; ++c) {
        if (v[c] != 0) {
          CHECK(a[c] == 0);
          CHECK(b[c] == 0);
        }
      }
    }

    for (Vector3f v : {Vector3f(0.00038527316, 0.00038460016, -0.99999988079),
                       Vector3f(-0.00019813581, -0.00008946839, -0.99999988079)}) {
      coordinate_system(v, &a, &b);
      CHECK(error(v, a, b) < 1e-10);
    }

    Rng rng;
    for (int i = 0; i < 1'000; ++i) {
      Point2f u = {rng.uniform<Float>(), rng.uniform<Float>()};
      Vector3f v = sample_uniform_sphere(u);
      coordinate_system(v, &a, &b);
      CHECK(error(v, a, b) < 1e-10);
    }
  }
}

TEST_CASE("Point3", "[util][vecmath]") {
  SECTION("Interval") {
    // This test is primarilly to confirm the Tuple3 Interval variants compile
    // successfully, it does not actually test any of the behavior currently.
    Point3fi p(1, 2, 3), q(6, 9, 1);
    Vector3fi v(4, 5, 6);

    p += v;
    p = (p - v);
    Vector3<Interval> scaled = Interval(4) * v;
    p = static_cast<Point3<Interval>>(p) + scaled;
    Interval d = dot(v, v);
    d = distance_squared(p, q);
    d = distance(p, q);

    Vector3fi vv = cross(v, v);
  }
}
