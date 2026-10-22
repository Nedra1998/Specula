#include "specula/util/vecmath/bounds3.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Bounds3", "[util][vecmath]") {
  SECTION("Point distance") {
    Bounds3f b(Point3f(0, 0, 0), Point3f(1, 1, 1));

    CHECK(distance(Point3f(0.5, 0.5, 0.5), b) == 0.0);
    CHECK(distance(Point3f(0.0, 1.0, 1.0), b) == 0.0);
    CHECK(distance(Point3f(0.25, 0.8, 1.0), b) == 0.0);
    CHECK(distance(Point3f(0.0, 0.25, 0.8), b) == 0.0);
    CHECK(distance(Point3f(0.7, 0.0, 0.8), b) == 0.0);

    CHECK(distance(Point3f(6.0, 1.0, 1.0), b) == 5.0);
    CHECK(distance(Point3f(0.0, -10.0, 1.0), b) == 10.0);

    CHECK(distance(Point3f(0.5, 0.5, 3.0), b) == 2.0);
    CHECK(distance(Point3f(0.5, 0.5, -3.0), b) == 3.0);
    CHECK(distance(Point3f(0.5, 3.0, 0.5), b) == 2.0);
    CHECK(distance(Point3f(0.5, -3.0, 0.5), b) == 3.0);
    CHECK(distance(Point3f(3.0, 0.5, 0.5), b) == 2.0);
    CHECK(distance(Point3f(-3.0, 0.5, 0.5), b) == 3.0);

    CHECK(distance_squared(Point3f(4, 8, -10), b) == 3 * 3 + 7 * 7 + 10 * 10);
    CHECK(distance_squared(Point3f(-6, -10, 8), b) == 6 * 6 + 10 * 10 + 7 * 7);

    Bounds3f b2(Point3f(-1, -3, 5), Point3f(2, -2, 18));
    CHECK(distance(Point3f(-0.99, -2.0, 5.0), b2) == 0.0);
    CHECK(distance_squared(Point3f(-3, -9, 22), b2) == 2 * 2 + 6 * 6 + 4 * 4);
  }

  SECTION("Union") {
    Bounds3f a(Point3f(-10, -10, 5), Point3f(0, 20, 10));
    Bounds3f b;
    Bounds3f c = bunion(a, b);
    CHECK(a == c);

    CHECK(b == bunion(b, b));

    Bounds3f d(Point3f(-15, 10, 30));
    Bounds3f e = bunion(a, d);
    CHECK(Bounds3f(Point3f(-15, -10, 5), Point3f(0, 20, 30)) == e);
  }
}
