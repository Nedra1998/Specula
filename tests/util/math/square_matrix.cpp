#include "specula/util/math/square_matrix.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <specula/util/vecmath/tuple3.hpp>

#include "formatter.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("SquareMatrix", "[util][math]") {
  SECTION("Basic Operations 2x2") {
    SquareMatrix<2> m2;

    CHECK(m2.is_identity());

    CHECK(m2 == SquareMatrix<2>(1, 0, 0, 1));
    CHECK(m2 != SquareMatrix<2>(0, 1, 1, 0));

    CHECK(SquareMatrix<2>(2, 0, 0, -1) == SquareMatrix<2>::diag(2, -1));

    SquareMatrix<2> m(1, 2, 3, 4);
    CHECK(!m.is_identity());
    SquareMatrix<2> mt(1, 3, 2, 4);
    CHECK(transpose(m) == mt);

    pstd::array<Float, 2> v{1, -2};
    pstd::array<Float, 2> vt = m * v;
    CHECK(1 - 2 * 2 == vt[0]);
    CHECK(3 - 4 * 2 == vt[1]);

    pstd::optional<SquareMatrix<2>> inv = inverse(m2);
    REQUIRE(inv.has_value());
    CHECK(m2 == *inv);

    SquareMatrix<2> ms(2, 4, -4, 8);
    inv = inverse(ms);
    REQUIRE(inv.has_value());
    CHECK(SquareMatrix<2>(1.0 / 4.0, -1.0 / 8.0, 1.0 / 8.0, 1.0 / 16.0) == *inv);

    SquareMatrix<2> degen(0, 0, 2, 0);
    inv = inverse(degen);
    CHECK(!inv.has_value());
  }

  SECTION("Basic Operations 3x3") {
    SquareMatrix<3> m3;
    CHECK(m3.is_identity());

    CHECK(m3 == SquareMatrix<3>(1, 0, 0, 0, 1, 0, 0, 0, 1));
    CHECK(m3 != SquareMatrix<3>(0, 1, 0, 0, 1, 0, 0, 0, 1));

    CHECK(SquareMatrix<3>(2, 0, 0, 0, -1, 0, 0, 0, 3) == SquareMatrix<3>::diag(2, -1, 3));

    SquareMatrix<3> m(1, 2, 3, 4, 5, 6, 7, 8, 9);
    CHECK(!m.is_identity());
    SquareMatrix<3> mt(1, 4, 7, 2, 5, 8, 3, 6, 9);
    CHECK(transpose(m) == mt);

    pstd::array<Float, 3> v{1, -2, 4};
    pstd::array<Float, 3> vt = m * v;
    CHECK(1 - 4 + 12 == vt[0]);
    CHECK(4 - 10 + 24 == vt[1]);
    CHECK(7 - 16 + 36 == vt[2]);

    Vector3f v3(1, -2, 4);
    Vector3f vt3 = m * v3;
    CHECK(1 - 4 + 12 == vt3[0]);
    CHECK(4 - 10 + 24 == vt3[1]);
    CHECK(7 - 16 + 36 == vt3[2]);

    pstd::optional<SquareMatrix<3>> inv = inverse(m3);
    REQUIRE(inv.has_value());
    CHECK(m3 == *inv);

    SquareMatrix<3> ms(2, 0, 0, 0, 4, 0, 0, 0, -1);
    inv = inverse(ms);
    REQUIRE(inv.has_value());
    CHECK(SquareMatrix<3>(0.5, 0, 0, 0, 0.25, 0, 0, 0, -1) == *inv);

    SquareMatrix<3> degen(0, 0, 2, 0, 0, 0, 1, 1, 1);
    inv = inverse(degen);
    CHECK(!inv.has_value());
  }

  SECTION("Basic Operations 4x4") {
    SquareMatrix<4> m4;
    CHECK(m4.is_identity());

    CHECK(m4 == SquareMatrix<4>(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
    CHECK(m4 != SquareMatrix<4>(0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0));

    SquareMatrix<4> diag(8, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.5);
    CHECK(diag == SquareMatrix<4>::diag(8, 2, 1, 0.5));

    SquareMatrix<4> m(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    CHECK(!m.is_identity());
    SquareMatrix<4> mt(1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16);
    CHECK(transpose(m) == mt);

    pstd::optional<SquareMatrix<4>> inv = inverse(m4);
    REQUIRE(inv.has_value());
    CHECK(m4 == *inv);

    inv = inverse(diag);
    REQUIRE(inv.has_value());
    CHECK(SquareMatrix<4>::diag(.125, .5, 1, 2) == *inv);

    SquareMatrix<4> degen(2, 0, 0, 0, 0, 4, 0, 0, 0, -3, 0, 1, 0, 0, 0, 0);
    inv = inverse(degen);
    CHECK(!inv.has_value());
  }
}
