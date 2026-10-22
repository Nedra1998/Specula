#include "specula/util/vecmath/equal_area.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/sampling.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch;
using namespace Catch::Matchers;

TEST_CASE("Equal area", "[util][vecmath]") {
  SECTION("Randoms") {
    for (Point2f u : Uniform2D(100)) {
      Vector3f v = sample_uniform_sphere(u);
      Point2f c = equal_area_sphere_to_square(v);
      Vector3f vp = equal_area_square_to_sphere(c);
      CAPTURE(v, c, vp);
      CHECK_THAT(length(vp), WithinAbs(1.0, 0.0001));
      CHECK(dot(v, vp) > 0.9999);
    }
  }

  SECTION("Remap Edges") {
    auto check_close = [&](Point2f a, Point2f b) {
      Vector3f av = equal_area_square_to_sphere(a);
      b = wrap_equal_area_square(b);
      Vector3f bv = equal_area_square_to_sphere(b);
      CHECK(dot(av, bv) > 0.99);
    };

    check_close(Point2f(.25, .01), Point2f(.25, -.01));
    check_close(Point2f(.89, .01), Point2f(.89, -.01));

    check_close(Point2f(.25, .99), Point2f(.25, 1.01));
    check_close(Point2f(.89, .99), Point2f(.89, 1.01));

    check_close(Point2f(.01, .66), Point2f(-.01, .66));
    check_close(Point2f(.01, .15), Point2f(-.01, .15));

    check_close(Point2f(.99, .66), Point2f(1.01, .66));
    check_close(Point2f(.99, .15), Point2f(1.01, .15));

    check_close(Point2f(.01, .01), Point2f(-.01, -.01));
    check_close(Point2f(.99, .01), Point2f(1.01, -.01));
    check_close(Point2f(.01, .99), Point2f(-.01, 1.01));
    check_close(Point2f(.99, .99), Point2f(1.01, 1.01));
  }
}
