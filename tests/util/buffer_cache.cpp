#include "specula/util/buffer_cache.hpp"

#include <vector>

#include <catch2/catch_test_macros.hpp>

using namespace specula;

TEST_CASE("BufferCache", "[util]") {
  SECTION("Basics") {
    REQUIRE(INT_BUFFER_CACHE != nullptr);
    std::vector<int> v{1, 2, 3, 4, 5};
    size_t base_mem = INT_BUFFER_CACHE->bytes_used();

    Allocator alloc;
    const int *ptr = INT_BUFFER_CACHE->lookup_or_add(v, alloc);
    for (size_t i = 0; i < v.size(); ++i) {
      CHECK(ptr[i] == v[i]);
    }
    CHECK(5 * sizeof(int) == INT_BUFFER_CACHE->bytes_used() - base_mem);

    CHECK(ptr == INT_BUFFER_CACHE->lookup_or_add(v, alloc));
    CHECK(5 * sizeof(int) == INT_BUFFER_CACHE->bytes_used() - base_mem);

    std::vector<int> v2{1, 2, 3, 4};
    const int *ptr2 = INT_BUFFER_CACHE->lookup_or_add(v2, alloc);
    CHECK(ptr != ptr2);
    for (size_t i = 0; i < v2.size(); ++i) {
      CHECK(ptr2[i] == v2[i]);
    }
    CHECK(9 * sizeof(int) == INT_BUFFER_CACHE->bytes_used() - base_mem);
  }
}
