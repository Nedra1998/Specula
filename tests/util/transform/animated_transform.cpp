#include "specula/util/transform/animated_transform.hpp"

#include <catch2/catch_test_macros.hpp>

#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"
#include "specula/util/transform/transform.hpp"

using namespace specula;

static Transform random_transform(Rng &rng) {
  Transform t;
  auto r = [&rng]() { return -10.0 + 20.0 * rng.uniform<Float>(); };
  for (int i = 0; i < 10; ++i) {
    switch (rng.uniform<uint32_t>(3)) {
    case 0:
      t = t * scale(std::abs(r()), std::abs(r()), std::abs(r()));
      break;
    case 1:
      t = t * translate(Vector3f(r(), r(), r()));
      break;
    case 2:
      t = t *
          rotate(r() * 20.0, sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()}));
      break;
    default:
      break;
    }
  }
  return t;
}

TEST_CASE("AnimatedTransform", "[util][transform]") {
  SECTION("Randoms") {
    Rng rng;
    auto r = [&rng]() { return -10.0 + 20.0 * rng.uniform<Float>(); };

    for (int i = 0; i < 200; ++i) {
      auto t0 = random_transform(rng);
      auto t1 = random_transform(rng);
      AnimatedTransform at(t0, 0.0, t1, 1.0);

      CAPTURE(i, t0, t1, at);
      for (int j = 0; j < 5; ++j) {
        Bounds3f bounds(Point3f(r(), r(), r()), Point3f(r(), r(), r()));
        Bounds3f motion_bounds = at.motion_bounds(bounds);

        for (Float t = 0.0; t <= 1.0; t += 1e-2 * rng.uniform<Float>()) {
          Transform tr = at.interpolate(t);
          Bounds3f tb = tr(bounds);
          tb.p_min += static_cast<Float>(1e-4) * tb.diagonal();
          tb.p_max -= static_cast<Float>(1e-4) * tb.diagonal();

          CHECK(tb.p_min.x >= motion_bounds.p_min.x);
          CHECK(tb.p_max.x <= motion_bounds.p_max.x);
          CHECK(tb.p_min.y >= motion_bounds.p_min.y);
          CHECK(tb.p_max.y <= motion_bounds.p_max.y);
          CHECK(tb.p_min.z >= motion_bounds.p_min.z);
          CHECK(tb.p_max.z <= motion_bounds.p_max.z);
        }
      }
    }
  }
}
