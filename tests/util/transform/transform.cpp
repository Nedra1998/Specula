#include "specula/util/transform/transform.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("RotateFromTo", "[util][transform]") {
  SECTION("Simple") {
    {
      Vector3f from(0, 0, 1), to(0, 0, 1);
      Transform r = rotate_from_to(from, to);
      CHECK(to == r(from));
    }

    {
      Vector3f from(0, 0, 1), to(1, 0, 0);
      Transform r = rotate_from_to(from, to);
      CHECK(to == r(from));
    }

    {
      Vector3f from(0, 0, 1), to(0, 1, 0);
      Transform r = rotate_from_to(from, to);
      CHECK(to == r(from));
    }
  }

  SECTION("Randoms") {
    Rng rng;
    for (int i = 0; i < 100; ++i) {
      Vector3f from = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});
      Vector3f to = sample_uniform_sphere({rng.uniform<Float>(), rng.uniform<Float>()});

      Transform r = rotate_from_to(from, to);
      Vector3f to_new = r(from);
      CHECK_THAT(length(to_new), WithinAbs(1.0, 1e-3f));
      CHECK(dot(to, to_new) > 0.999f);
    }
  }
}
