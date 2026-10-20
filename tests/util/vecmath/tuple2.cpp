#include "specula/util/vecmath/tuple2.hpp"

#include <catch2/catch_test_macros.hpp>

#include "specula/util/rng.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;

TEST_CASE("Vector2", "[util][vecmath]") {
  SECTION("Basics") {
    Vector2f vf(-1, 10);

    CHECK(vf == Vector2f(Vector2i(-1, 10)));
    CHECK(vf != Vector2f(-1, 100));

    CHECK(vf + vf == Vector2f(-2, 20));
    CHECK(vf - vf == Vector2f(0, 0));
    CHECK(vf * 2 == Vector2f(-2, 20));
    CHECK(2 * vf == Vector2f(-2, 20));
    CHECK(vf / 2 == Vector2f(-0.5, 5));

    CHECK(abs(vf) == Vector2f(1, 10));

    CHECK(vf == ceil(Vector2f(-1.5, 9.9)));
    CHECK(vf == floor(Vector2f(-0.5, 10.01)));

    CHECK(min(vf, Vector2f(-20, 20)) == Vector2f(-20, 10));
    CHECK(max(vf, Vector2f(-20, 20)) == Vector2f(-1, 20));

    CHECK(min_component_value(vf) == -1);
    CHECK(min_component_value(-vf) == -10);
    CHECK(max_component_value(vf) == 10);
    CHECK(max_component_value(-vf) == 1);

    CHECK(min_component_index(vf) == 0);
    CHECK(min_component_index(-vf) == 1);
    CHECK(max_component_index(vf) == 1);
    CHECK(max_component_index(-vf) == 0);

    CHECK(permute(vf, {0, 1}) == vf);
    CHECK(permute(vf, {1, 0}) == Vector2f(10, -1));
    CHECK(permute(vf, {1, 1}) == Vector2f(10, 10));
  }
}

TEST_CASE("Point2", "[util][vecmath]") {
  SECTION("Invert bilinear") {
    auto bilerp = [](Point2f u, const Point2f *v) {
      return lerp(u[0], lerp(u[1], v[0], v[2]), lerp(u[1], v[1], v[3]));
    };

    Rng rng;
    Point2f v_real_simple[4] = {Point2f(0, 0), Point2f(1, 0), Point2f(0, 1), Point2f(1, 1)};
    for (int i = 0; i < 100; ++i) {
      Point2f u{rng.uniform<Float>(), rng.uniform<Float>()};
      Point2f up = bilerp(u, v_real_simple);
      CHECK(distance(u, up) < 1e-5f);
      CHECK(distance(u, invert_bilinear(up, v_real_simple)) < 1e-5f);
    }

    Point2f v_simple[4] = {Point2f(10, -4), Point2f(10, -9), Point2f(15, -4), Point2f(15, -9)};
    for (int i = 0; i < 100; ++i) {
      Point2f u{rng.uniform<Float>(), rng.uniform<Float>()};
      Point2f up = bilerp(u, v_simple);
      Point2f uinv = invert_bilinear(up, v_simple);
      CHECK(distance(u, uinv) < 1e-5f);
    }

    for (int i = 0; i < 100; ++i) {
      for (int j = 0; j < 100; ++j) {
        Point2f u{rng.uniform<Float>(), rng.uniform<Float>()};
        Point2f up = bilerp(u, v_simple);
        Point2f uinv = invert_bilinear(up, v_simple);
        CHECK(distance(u, uinv) < 1e-5f);
      }
    }
  }
}
