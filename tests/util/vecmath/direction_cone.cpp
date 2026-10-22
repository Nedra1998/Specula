#include "specula/util/vecmath/direction_cone.hpp"

#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

specula::DirectionCone random_cone(specula::Rng &rng) {
  specula::Vector3f w =
      specula::sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
  return {w, -1 + 2 * rng.uniform<Float>()};
}

TEST_CASE("DirectionCone", "[util][vecmath]") {
  SECTION("Union basics") {
    DirectionCone c = bunion(DirectionCone(Vector3f(0, 0, 1), std::cos(PI / 2)),
                             DirectionCone(Vector3f(0.1, 0.1, 0.1), std::cos(0.1)));
    CHECK(c.w == Vector3f(0, 0, 1));
    CHECK(c.cos_theta == std::cos(PI / 2));

    c = bunion(DirectionCone(Vector3f(0.1, 0.1, 0.1), std::cos(0.1)),
               DirectionCone(Vector3f(0, 0, 1), std::cos(PI / 2)));
    CHECK(c.w == Vector3f(0, 0, 1));
    CHECK(c.cos_theta == std::cos(PI / 2));

    Vector3f w(1, 0.5, -0.25);
    c = bunion(DirectionCone(w, std::cos(0.12)), DirectionCone(w, std::cos(0.03)));
    CHECK(c.w == normalize(w));
    CHECK(c.cos_theta == std::cos(Float(0.12)));

    c = bunion(DirectionCone(w, std::cos(0.1)), DirectionCone(w, std::cos(0.2)));
    CHECK(c.w == normalize(w));
    CHECK(c.cos_theta == std::cos(Float(0.2)));

    c = bunion(DirectionCone(Vector3f(-1, -1, -1), std::cos(1.01 * PI / 2)),
               DirectionCone(Vector3f(1.001, 1, 1), std::cos(1.01 * PI / 2)));
    CHECK(c.cos_theta == std::cos(PI));

    c = bunion(DirectionCone(Vector3f(1, 0, 0), std::cos(1e-3)),
               DirectionCone(Vector3f(0, 1, 0), std::cos(1e-3)));
    CHECK_THAT(dot(c.w, normalize(Vector3f(1, 1, 0))), WithinRel(Float(1.0)));
    CHECK_THAT(c.cos_theta, WithinAbs(std::cos((PI / 2 + 2e-3) / 2), 1e-3));
  }

  SECTION("Union randoms") {
    Rng rng(16);

    for (int i = 0; i < 100; ++i) {
      DirectionCone a = random_cone(rng), b = random_cone(rng);
      DirectionCone c = bunion(a, b);

      for (int j = 0; j < 100; ++j) {
        Vector3f w = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
        if (inside(a, w) || inside(b, w)) {
          CAPTURE(fmt::format("{}", a), fmt::format("{}", b), fmt::format("{}", c),
                  fmt::format("{}", w), inside(a, w), inside(b, w));
          CHECK(inside(c, w));
        }
      }
    }
  }

  SECTION("Bound bounds") {
    Bounds3f b(Point3f(0, 0, 0), Point3f(1, 1, 1));

    DirectionCone c = bound_subtended_directions(b, Point3f(0.1, 0.2, 0.3));
    CHECK(c.cos_theta == std::cos(PI));

    c = bound_subtended_directions(b, Point3f(-0.5, 0.5, 0.5));
    Float theta = std::acos(c.cos_theta);
    Float precise = std::atan(std::sqrt(0.5 * 0.5 + 0.5 * 0.5) / 0.5);
    CHECK(theta >= 1.0001 * precise);
    CHECK(theta < 1.1 * precise);

    Rng rng(512);
    for (int i = 0; i < 1000; ++i) {
      Bounds3f b(Point3f(lerp(rng.uniform<Float>(), -1, 1), lerp(rng.uniform<Float>(), -1, 1),
                         lerp(rng.uniform<Float>(), -1, 1)),
                 Point3f(lerp(rng.uniform<Float>(), -1, 1), lerp(rng.uniform<Float>(), -1, 1),
                         lerp(rng.uniform<Float>(), -1, 1)));

      Point3f p(lerp(rng.uniform<Float>(), -4, 4), lerp(rng.uniform<Float>(), -4, 4),
                lerp(rng.uniform<Float>(), -4, 4));

      c = bound_subtended_directions(b, p);
      if (inside(p, b)) {
        CHECK(c.cos_theta == std::cos(PI));
      } else {
        Vector3f wx, wy;
        coordinate_system(c.w, &wx, &wy);
        for (int j = 0; j < 1000; ++j) {
          Point2f u(rng.uniform<Float>(), rng.uniform<Float>());
          Vector3f w = sample_uniform_sphere(u);
          // TODO: Uncomment when the Ray class has been implemented
          // Ray r(p, w);
          // bool hit = b.intersect_p(r.o, r.d);
          // if (hit) {
          //   CHECK(inside(c, w));
          // }
          // if (!inside(c, w)) {
          //   CHECK_FALSE(hit);
          // }
        }
      }
    }
  }

  SECTION("Vector in cone") {
    Rng rng;

    for (int i = 0; i < 100; ++i) {
      DirectionCone dc = random_cone(rng);

      for (int j = 0; j < 100; ++j) {
        Vector3f wrandom = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
        Vector3f wclosest = dc.closest_vector_in_cone(wrandom);

        if (inside(dc, wrandom)) {
          CHECK(dot(wclosest, wrandom) > 0.999);
        } else {
          Float sin_theta = safe_sqrt(1 - dc.cos_theta * dc.cos_theta);
          Frame f = Frame::from_z(dc.w);

          Vector3f wbest;
          Float best_dot = -1;
          const int nk = 1000;
          for (int k = 0; k < nk; ++k) {
            Float phi = (k + 0.5) / nk * 2 * PI;
            Vector3f w = spherical_direction(sin_theta, dc.cos_theta, phi);
            w = f.from_local(w);
            if (dot(w, wrandom) > best_dot) {
              wbest = w;
              best_dot = dot(w, wrandom);
            }
          }
          CHECK(dot(wbest, wclosest) > 0.999);
        }
      }
    }
  }
}
