#include <catch2/catch_test_macros.hpp>
#include <specula/util/pstd/optional.hpp>

using namespace specula;

struct AliveCounter {
  AliveCounter() { ++count; }
  AliveCounter(const AliveCounter &c) { ++count; }
  AliveCounter(AliveCounter &&c) { ++count; }

  ~AliveCounter() { --count; }

  AliveCounter &operator=(const AliveCounter &c) {
    ++count;
    return *this;
  }
  AliveCounter &operator=(AliveCounter &&c) {
    ++count;
    return *this;
  }

  static int count;
};
int AliveCounter::count;

TEST_CASE("Optional", "[util][pstd]") {
  SECTION("Basics") {
    pstd::optional<int> opt;

    CHECK(!opt.has_value());

    opt = 1;
    CHECK(opt.has_value());
    CHECK((bool)opt);
    CHECK(opt.value() == 1);
    CHECK(*opt == 1);

    opt.reset();
    CHECK(!opt.has_value());
    CHECK(!(bool)opt);
    CHECK(opt.value_or(2) == 2);

    opt = 1;
    CHECK(opt.value_or(2) == 1);

    int x = 3;
    opt.reset();
    opt = std::move(x);
    CHECK(opt.has_value());
    CHECK(opt.value() == 3);
  }

  SECTION("Runtime Destruction") {
    AliveCounter::count = 0;

    pstd::optional<AliveCounter> opt;
    CHECK(AliveCounter::count == 0);

    opt = AliveCounter();
    CHECK(AliveCounter::count == 1);

    opt.reset();
    CHECK(AliveCounter::count == 0);

    {
      pstd::optional<AliveCounter> opt2 = AliveCounter();
      CHECK(AliveCounter::count == 1);
    }
    CHECK(AliveCounter::count == 0);

    {
      AliveCounter ac2;
      CHECK(AliveCounter::count == 1);
      opt.reset();
      CHECK(AliveCounter::count == 1);
      opt = std::move(ac2);
      CHECK(AliveCounter::count == 2);
    }
    CHECK(AliveCounter::count == 1);

    opt.reset();
    CHECK(AliveCounter::count == 0);

    {
      AliveCounter ac2;
      CHECK(AliveCounter::count == 1);
      pstd::optional<AliveCounter> opt2(std::move(ac2));
      CHECK(AliveCounter::count == 2);

      opt2.reset();
      CHECK(AliveCounter::count == 1);
    }

    CHECK(AliveCounter::count == 0);
  }
}
