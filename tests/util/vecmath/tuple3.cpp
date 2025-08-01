// WARNING: This file was generated using AI and may contain inaccuracies or errors.
// It should be reviewed and tested thoroughly before use in production.

#include "specula/util/vecmath/tuple3.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Tuple3", "[util][vecmath]") {
  SECTION("Basic operations") {
    Vector3f v1(1.0f, 2.0f, 3.0f);
    Vector3f v2(4.0f, 5.0f, 6.0f);

    // Test basic operations
    CHECK(v1 + v2 == Vector3f(5.0f, 7.0f, 9.0f));
    CHECK(v2 - v1 == Vector3f(3.0f, 3.0f, 3.0f));
    CHECK(v1 * 2.0f == Vector3f(2.0f, 4.0f, 6.0f));
    CHECK(2.0f * v1 == Vector3f(2.0f, 4.0f, 6.0f));
    CHECK(v2 / 2.0f == Vector3f(2.0f, 2.5f, 3.0f));

    // Test compound assignment
    Vector3f v3 = v1;
    v3 += v2;
    CHECK(v3 == Vector3f(5.0f, 7.0f, 9.0f));

    v3 = v2;
    v3 -= v1;
    CHECK(v3 == Vector3f(3.0f, 3.0f, 3.0f));

    v3 = v1;
    v3 *= 2.0f;
    CHECK(v3 == Vector3f(2.0f, 4.0f, 6.0f));

    v3 = v2;
    v3 /= 2.0f;
    CHECK(v3 == Vector3f(2.0f, 2.5f, 3.0f));

    // Test equality operators
    CHECK(v1 == Vector3f(1.0f, 2.0f, 3.0f));
    CHECK(v1 != v2);

    // Test negation
    CHECK(-v1 == Vector3f(-1.0f, -2.0f, -3.0f));

    // Test indexing
    CHECK(v1[0] == 1.0f);
    CHECK(v1[1] == 2.0f);
    CHECK(v1[2] == 3.0f);
  }

  SECTION("Helper functions") {
    Vector3f v1(-1.5f, 2.3f, -3.7f);
    Vector3f v2(3.2f, 1.0f, 4.5f);

    // Test abs
    CHECK(abs(v1) == Vector3f(1.5f, 2.3f, 3.7f));

    // Test ceil and floor
    CHECK(ceil(v1) == Vector3f(-1.0f, 3.0f, -3.0f));
    CHECK(floor(v1) == Vector3f(-2.0f, 2.0f, -4.0f));

    // Test lerp
    Vector3f lerp_result = lerp(0.5f, v1, v2);
    CHECK_THAT(lerp_result.x, WithinAbs(0.85f, 0.00001f));
    CHECK_THAT(lerp_result.y, WithinAbs(1.65f, 0.00001f));
    CHECK_THAT(lerp_result.z, WithinAbs(0.4f, 0.00001f));

    // Test min/max
    CHECK(min(v1, v2) == Vector3f(-1.5f, 1.0f, -3.7f));
    CHECK(max(v1, v2) == Vector3f(3.2f, 2.3f, 4.5f));

    // Test component functions
    CHECK(min_component_value(v1) == -3.7f);
    CHECK(max_component_value(v1) == 2.3f);
    CHECK(min_component_index(v1) == 2); // z is -3.7f
    CHECK(max_component_index(v1) == 1); // y is 2.3f

    // Test permute
    CHECK(permute(v1, {1, 2, 0}) == Vector3f(2.3f, -3.7f, -1.5f));

    // Test horizontal product
    CHECK_THAT(hprod(v1), WithinAbs(12.765f, 0.00001f));
  }
}

TEST_CASE("Vector3", "[util][vecmath]") {
  SECTION("Constructors and conversions") {
    // Basic constructor
    Vector3f v1(1.0f, 2.0f, 3.0f);
    CHECK(v1.x == 1.0f);
    CHECK(v1.y == 2.0f);
    CHECK(v1.z == 3.0f);

    // Type conversion constructor
    Vector3i vi(3, 4, 5);
    Vector3f vf(vi);
    CHECK(vf.x == 3.0f);
    CHECK(vf.y == 4.0f);
    CHECK(vf.z == 5.0f);

    // Point to Vector conversion
    Point3f p(5.0f, 6.0f, 7.0f);
    Vector3f vp(p);
    CHECK(vp.x == 5.0f);
    CHECK(vp.y == 6.0f);
    CHECK(vp.z == 7.0f);

    // Normal to Vector conversion
    Normal3f n(1.0f, 2.0f, 3.0f);
    Vector3f vn(n);
    CHECK(vn.x == 1.0f);
    CHECK(vn.y == 2.0f);
    CHECK(vn.z == 3.0f);
  }
}

TEST_CASE("Point3", "[util][vecmath]") {
  SECTION("Basic operations") {
    Point3f p1(1.0f, 2.0f, 3.0f);
    Point3f p2(4.0f, 5.0f, 6.0f);
    Vector3f v(2.0f, 3.0f, 4.0f);

    // Test addition with vector
    CHECK(p1 + v == Point3f(3.0f, 5.0f, 7.0f));

    // Test compound addition with vector
    Point3f p3 = p1;
    p3 += v;
    CHECK(p3 == Point3f(3.0f, 5.0f, 7.0f));

    // Test subtraction of points (yielding a vector)
    Vector3f v_diff = p2 - p1;
    CHECK(v_diff == Vector3f(3.0f, 3.0f, 3.0f));

    // Test point minus vector (yielding a point)
    CHECK(p2 - v == Point3f(2.0f, 2.0f, 2.0f));

    // Test compound subtraction with vector
    p3 = p2;
    p3 -= v;
    CHECK(p3 == Point3f(2.0f, 2.0f, 2.0f));

    // Test negation
    CHECK(-p1 == Point3f(-1.0f, -2.0f, -3.0f));

    // Test scalar multiplication
    CHECK(p1 * 2.0f == Point3f(2.0f, 4.0f, 6.0f));
    CHECK(2.0f * p1 == Point3f(2.0f, 4.0f, 6.0f));

    // Test compound scalar multiplication
    p3 = p1;
    p3 *= 2.0f;
    CHECK(p3 == Point3f(2.0f, 4.0f, 6.0f));
  }

  SECTION("Conversions") {
    // Convert between types
    Point3i pi(1, 2, 3);
    Point3f pf(pi);
    CHECK(pf.x == 1.0f);
    CHECK(pf.y == 2.0f);
    CHECK(pf.z == 3.0f);

    // Vector to Point conversion
    Vector3f v(3.0f, 4.0f, 5.0f);
    Point3f p(v);
    CHECK(p.x == 3.0f);
    CHECK(p.y == 4.0f);
    CHECK(p.z == 5.0f);
  }
}

TEST_CASE("Normal3", "[util][vecmath]") {
  SECTION("Constructors and conversions") {
    // Basic constructor
    Normal3f n1(1.0f, 2.0f, 3.0f);
    CHECK(n1.x == 1.0f);
    CHECK(n1.y == 2.0f);
    CHECK(n1.z == 3.0f);

    // Normal from Normal constructor
    Normal3f n2(Normal3f(4.0f, 5.0f, 6.0f));
    CHECK(n2.x == 4.0f);
    CHECK(n2.y == 5.0f);
    CHECK(n2.z == 6.0f);

    // Vector to Normal conversion
    Vector3f v(3.0f, 4.0f, 5.0f);
    Normal3f n3(v);
    CHECK(n3.x == 3.0f);
    CHECK(n3.y == 4.0f);
    CHECK(n3.z == 5.0f);
  }

  SECTION("Basic operations") {
    Normal3f n1(1.0f, 2.0f, 3.0f);
    Normal3f n2(4.0f, 5.0f, 6.0f);

    // Test addition
    CHECK(n1 + n2 == Normal3f(5.0f, 7.0f, 9.0f));

    // Test scalar operations
    CHECK(n1 * 2.0f == Normal3f(2.0f, 4.0f, 6.0f));
    CHECK(2.0f * n1 == Normal3f(2.0f, 4.0f, 6.0f));

    // Test compound scalar multiplication
    Normal3f n3 = n1;
    n3 *= 2.0f;
    CHECK(n3 == Normal3f(2.0f, 4.0f, 6.0f));
  }
}
