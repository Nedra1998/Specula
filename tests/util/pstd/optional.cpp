#include "specula/util/pstd/optional.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace specula::pstd;

struct AliveCounter {
  AliveCounter() { ++n_alive; }
  AliveCounter(const AliveCounter &c) { ++n_alive; }
  AliveCounter(AliveCounter &&c) noexcept { ++n_alive; }
  AliveCounter &operator=(const AliveCounter &c) {
    ++n_alive;
    return *this;
  }
  AliveCounter &operator=(AliveCounter &&c) noexcept {
    ++n_alive;
    return *this;
  }

  ~AliveCounter() { --n_alive; }

  static int n_alive;
};
int AliveCounter::n_alive;

TEST_CASE("Optional", "[util][pstd]") {
  SECTION("Basics") {
    optional<int> opt;

    CHECK_FALSE(opt.has_value());

    opt = 1;
    CHECK(opt.has_value());
    CHECK((bool)opt);
    CHECK(1 == opt.value());
    CHECK(1 == *opt);

    opt.reset();
    CHECK_FALSE(opt.has_value());
    CHECK_FALSE((bool)opt);
    CHECK(2 == opt.value_or(2));
    opt = 1;
    CHECK(1 == opt.value_or(2));

    int x = 3;
    opt.reset();
    opt = std::move(x);
    CHECK(opt.has_value());
    CHECK(3 == opt.value());
  }

  SECTION("Run destructors") {
    AliveCounter::n_alive = 0;

    optional<AliveCounter> opt;
    CHECK(0 == AliveCounter::n_alive);

    opt = AliveCounter();
    CHECK(1 == AliveCounter::n_alive);

    opt = AliveCounter();
    CHECK(1 == AliveCounter::n_alive);

    opt.reset();
    CHECK(0 == AliveCounter::n_alive);

    {
      optional<AliveCounter> opt2 = AliveCounter();
      CHECK(1 == AliveCounter::n_alive);
    }
    CHECK(0 == AliveCounter::n_alive);

    {
      AliveCounter ac2;
      CHECK(1 == AliveCounter::n_alive);

      opt.reset();
      CHECK(1 == AliveCounter::n_alive);

      opt = std::move(ac2);
      CHECK(2 == AliveCounter::n_alive);
    }
    CHECK(1 == AliveCounter::n_alive);

    opt.reset();
    CHECK(0 == AliveCounter::n_alive);

    {
      AliveCounter ac2;
      CHECK(1 == AliveCounter::n_alive);
      optional<AliveCounter> opt2(std::move(ac2));
      CHECK(2 == AliveCounter::n_alive);

      opt2.reset();
      CHECK(1 == AliveCounter::n_alive);
    }
    CHECK(0 == AliveCounter::n_alive);
  }
}
