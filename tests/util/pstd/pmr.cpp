#include <map>

#include <catch2/catch_test_macros.hpp>
#include <specula/util/pstd/pmr.hpp>

#include "specula/util/rng.hpp"

using namespace specula;

class TrackingResource : public pstd::pmr::memory_resource {
public:
  void *do_allocate(size_t bytes, size_t alignment) {
    void *ptr = new char[bytes];
    allocs[ptr] = bytes;
    return ptr;
  }

  void do_deallocate(void *ptr, size_t bytes, size_t alignment) {
    auto iter = allocs.find(ptr);
    REQUIRE(iter != allocs.end());
    REQUIRE(iter->second == bytes);
    allocs.erase(iter);
    delete[] (char *)ptr;
  }

  bool do_is_equal(const memory_resource &other) const noexcept { return &other == this; }

  std::map<void *, size_t> allocs;
};

TEST_CASE("Monotonic Buffer Resource", "[util][pstd][pmr]") {
  TrackingResource tr;
  pstd::pmr::monotonic_buffer_resource mb(1024, &tr);
  Allocator alloc(&mb);
  Rng rng;

  struct Span {
    char *ptr;
    size_t size;
  };
  std::vector<Span> spans;

  for (int i = 0; i < 10000; ++i) {
    size_t size;
    if (rng.uniform<Float>() < .5f)
      size = 1 << rng.uniform<int>(12);
    else
      size = rng.uniform<int>(2048);

    char *p = (char *)alloc.allocate_bytes(size);
    for (const Span &s : spans)
      CHECK((p >= s.ptr + s.size || p + size <= s.ptr));
    spans.push_back(Span{p, size});
  }
}
