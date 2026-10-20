#include "specula/util/parallel/functions.hpp"

#include <catch2/catch_test_macros.hpp>

#include "specula/util/vecmath.hpp"

using namespace specula;

TEST_CASE("Parallel", "[util][parallel]") {
  SECTION("Basics") {
    std::atomic<int> counter{0};
    parallel_for(0, 1000, [&](int64_t) { ++counter; });
    CHECK(counter == 1000);

    counter = 0;
    parallel_for(10, 1010, [&](int64_t start, int64_t end) {
      CHECK(end > start);
      CHECK(start >= 10);
      CHECK(start < 1010);
      CHECK(end > 10);
      CHECK(end <= 1010);
      for (int64_t i = start; i < end; ++i) {
        ++counter;
      }
    });
    CHECK(counter == 1000);

    counter = 0;
    parallel_for_2d(Bounds2i{{0, 0}, {15, 14}}, [&](Point2i p) { ++counter; });
    CHECK(counter == 15 * 14);
  }

  SECTION("Do nothing") {
    std::atomic<int> counter{0};
    parallel_for(0, 0, [&](int64_t) { ++counter; });
    CHECK(counter == 0);

    counter = 0;
    parallel_for_2d(Bounds2i{{0, 0}, {0, 0}}, [&](Point2i p) { ++counter; });
    CHECK(counter == 0);
  }

  SECTION("For each thread") {
    std::atomic<int> count{running_threads()};
    for_each_thread([&count] { --count; });
    CHECK(count == 0);
  }
}
