#include "specula/util/containers/hash_map.hpp"

#include <set>

#include <catch2/catch_test_macros.hpp>

#include "specula/util/rng.hpp"

using namespace specula;

TEST_CASE("HashMap", "[util][containers]") {
  SECTION("Basics") {
    Allocator alloc;
    HashMap<int, std::string, std::hash<int>> map(alloc);

    map.insert(1, "foo");
    map.insert(10, "hello");
    map.insert(42, "test");

    CHECK(3 == map.size());
    CHECK(map.capacity() >= 3);

    CHECK(map.has_key(1));
    CHECK(map.has_key(10));
    CHECK(map.has_key(42));
    CHECK_FALSE(map.has_key(0));
    CHECK_FALSE(map.has_key(1024));

    CHECK("foo" == map[1]);
    CHECK("hello" == map[10]);
    CHECK("test" == map[42]);

    map.insert(10, "bar");
    CHECK(3 == map.size());
    CHECK(map.capacity() >= 3);
    CHECK("bar" == map[10]);
  }

  SECTION("Randoms") {
    Allocator alloc;
    HashMap<int, int, std::hash<int>> map(alloc);
    std::set<int> values;
    Rng rng(1234);

    for (int i = 0; i < 10'000; ++i) {
      int v = rng.uniform<int>();
      values.insert(v);
      map.insert(v, -v);
    }

    CHECK(map.size() == values.size());

    rng.set_sequence(1234);
    for (int i = 0; i < 10'000; ++i) {
      int v = rng.uniform<int>();
      REQUIRE(map.has_key(v));
      CHECK(map[v] == -v);
    }

    int n_visited = 0;
    for (auto iter = map.begin(); iter != map.end(); ++iter) {
      ++n_visited;

      CHECK(iter->first == -iter->second);
      int v = iter->first;
      auto siter = values.find(v);
      REQUIRE(siter != values.end());
      values.erase(siter);
    }

    CHECK(n_visited == 10'000);
    CHECK(values.size() == 0);
  }
}
