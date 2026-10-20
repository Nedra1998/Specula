#include "specula/util/containers/intern_cache.hpp"

#include <catch2/catch_test_macros.hpp>

#include "specula/util/math.hpp"

using namespace specula;

template <typename T> struct Set {};

TEST_CASE("InternCache", "[util][containers]") {
  SECTION("BasicString") {
    InternCache<std::string> cache;

    const std::string *one = cache.lookup(std::string("one"));
    CHECK(cache.size() == 1);
    CHECK("one" == *one);

    const std::string *two = cache.lookup(std::string("two"));
    CHECK(one != two);
    CHECK("two" == *two);

    const std::string *another_one = cache.lookup(std::string("one"));
    CHECK(cache.size() == 2);
    CHECK("one" == *another_one);
    CHECK(one == another_one);
  }

  SECTION("BadHash") {
    struct BadIntHash {
      size_t operator()(int i) { return i % 7; }
    };

    InternCache<int, BadIntHash> cache;

    int n = 10'000;
    std::vector<const int *> first_pass(n);
    for (int i = 0; i < n; ++i) {
      int ii = permutation_element(i, n, 0xfeed00f5);
      const int *p = cache.lookup(ii);
      CHECK(*p == ii);
      first_pass[ii] = p;
      CHECK(i + 1 == cache.size());
    }

    for (int i = 0; i < n; ++i) {
      int ii = permutation_element(i, n, 0xfeed00f5);
      const int *p = cache.lookup(ii);
      CHECK(*p == ii);
      CHECK(first_pass[ii] == p);
      CHECK(n == cache.size());
    }
  }
}
