#include "specula/util/containers/array2d.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Array2D", "[util][containers]") {
  SECTION("Basics") {
    const int nx = 4, ny = 9;
    Array2D<Float> a(nx, ny);

    CHECK(nx == a.xsize());
    CHECK(ny == a.ysize());
    CHECK(nx * ny == a.size());

    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        a(x, y) = 1000 * x + y;
      }
    }

    for (int y = 0; y < ny; ++y) {
      for (int x = 0; x < nx; ++x) {
        CHECK(a(x, y) == 1000 * x + y);
      }
    }
  }

  SECTION("Bounds") {
    Bounds2i b(Point2i(-4, 3), Point2i(10, 7));
    Array2D<Point2f> a(b);

    CHECK(b.p_max.x - b.p_min.x == a.xsize());
    CHECK(b.p_max.y - b.p_min.y == a.ysize());

    for (Point2i p : b) {
      a[p] = Point2f(p.y, p.x);
    }
    for (Point2i p : b) {
      CHECK(a[p] == Point2f(p.y, p.x));
    }
  }
}
