#include "specula/util/vecmath/octahedral_vector.hpp"

#include <catch2/catch_get_random_seed.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/sampling.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("OctahedralVector", "[util][vecmath]") {
  SECTION("Encode decode") {
    for (Point2f u : Hammersley2D(65535)) {
      Vector3f v = sample_uniform_sphere(u);
      OctahedralVector ov(v);
      auto v2 = Vector3f(ov);
      CHECK_THAT(length(v2), WithinAbs(1.0, 0.001));
      CHECK_THAT(dot(v2, v), WithinAbs(1, 0.001));
    }
  }
}
