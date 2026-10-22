#include "specula/util/vecmath/spherical_geometry.hpp"

#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"
#include "specula/util/transform/transform.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("SphericalTriangleArea", "[util][vecmath]") {
  SECTION("Basics") {
    {
      Float a = spherical_triangle_area(Vector3f(1, 0, 0), Vector3f(0, 1, 0), Vector3f(0, 0, 1));
      CHECK_THAT(a, WithinAbs(PI / 2, 0.0001));
    }

    {
      Float a = spherical_triangle_area(Vector3f(1, 0, 0), normalize(Vector3f(1, 1, 0)),
                                        Vector3f(0, 0, 1));
      CHECK_THAT(a, WithinAbs(PI / 4, 0.0001));
    }

    Rng rng;
    for (int i = 0; i < 100; ++i) {
      Vector3f axis = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
      Float theta = 2 * PI * rng.uniform<Float>();
      Transform t = rotate(theta, axis);
      Vector3f va = t(Vector3f(1, 0, 0));
      Vector3f vb = t(Vector3f(0, 1, 0));
      Vector3f vc = t(Vector3f(0, 0, 1));
      Float a = spherical_triangle_area(va, vb, vc);
      CHECK_THAT(a, WithinAbs(PI / 2, 0.0001));
    }

    for (int i = 0; i < 100; ++i) {
      Vector3f axis = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
      Float theta = 2 * PI * rng.uniform<Float>();
      Transform t = rotate(theta, axis);
      Vector3f va = t(Vector3f(1, 0, 0));
      Vector3f vb = t(normalize(Vector3f(1, 1, 0)));
      Vector3f vc = t(Vector3f(0, 0, 1));
      Float a = spherical_triangle_area(va, vb, vc);
      CHECK_THAT(a, WithinAbs(PI / 4, 0.0001));
    }
  }

  SECTION("Random sampling") {
    for (int i = 0; i < 100; ++i) {
      Rng rng(i);
      Vector3f a = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
      Vector3f b = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
      Vector3f c = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});

      Vector3f axis = normalize(a + b + c);
      Frame frame = Frame::from_z(axis);
      Float cos_theta = std::min({dot(a, axis), dot(b, axis), dot(c, axis)});

      Float area = spherical_triangle_area(a, b, c);
      bool sample_sphere = area > PI;
      int sqrt_n = 200;
      int count = 0;

      for (Point2f u : Hammersley2D(sqrt_n * sqrt_n)) {
        Vector3f v;
        if (sample_sphere) {
          v = sample_uniform_sphere(u);
        } else {
          v = sample_uniform_cone(u, cos_theta);
          v = frame.from_local(v);
        }

        // TODO: Uncomment when the intersect_triangle method is implemented
        // if(intersect_triangle(Ray(Point3f(0, 0, 0), v), INFINITY, Point3f(a), Point3f(b),
        // Point3f(c))) {
        //   ++count;
        // }
      }

      Float pdf = sample_sphere ? uniform_sphere_pdf() : uniform_code_pdf(cos_theta);
      Float esta = static_cast<Float>(count) / (sqrt_n * sqrt_n * pdf);

      // CAPTURE(area, esta, a, b, c);
      // CHECK_THAT(esta, WithinRel(area, 0.035f));
    }
  }
}
