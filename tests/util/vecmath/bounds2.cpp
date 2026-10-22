#include "specula/util/vecmath/bounds2.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/macros.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Bounds2", "[util][vecmath]") {
  SECTION("Iterator basic") {
    Bounds2i b{{0, 1}, {2, 3}};
    Point2i e[] = {{0, 1}, {1, 1}, {0, 2}, {1, 2}};
    int offset = 0;
    for (auto p : b) {
      CHECK(offset < SPECULA_ARRAYSIZE(e));
      CHECK(e[offset] == p);
      ++offset;
    }
  }

  SECTION("Iterator degenerate") {
    Bounds2i b{{0, 0}, {0, 10}};
    for (auto p : b) {
      FAIL("This loop should never run");
    }

    Bounds2i b2{{0, 0}, {4, 0}};
    for (auto p : b2) {
      FAIL("This loop should never run");
    }

    Bounds2i b3;
    for (auto p : b3) {
      FAIL("This loop should never run");
    }
  }

  SECTION("Union") {
    Bounds2f a(Point2f(-10, -10), Point2f(0, 20));
    Bounds2f b;
    Bounds2f c = bunion(a, b);
    CHECK(a == c);

    CHECK(b == bunion(b, b));

    Bounds2f d(Point2f(-15, 10));
    Bounds2f e = bunion(a, d);
    CHECK(Bounds2f(Point2f(-15, -10), Point2f(0, 20)) == e);
  }
}
