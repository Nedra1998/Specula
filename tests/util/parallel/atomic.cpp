#include "specula/util/parallel/atomic.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace specula;

TEST_CASE("AtomicFloat", "[util][parallel]") {
  AtomicFloat af(0);
  Float f = 0.0;
  CHECK(f == af);
  af.add(1.0251);
  f += 1.0251;
  CHECK(f == af);
  af.add(2.0);
  f += 2.0;
  CHECK(f == af);
}
