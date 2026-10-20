#include "specula/util/hash.hpp"

#include <set>

#include <catch2/catch_test_macros.hpp>

using namespace specula;

TEST_CASE("Hash", "[util]") {
  SECTION("VarArgs") {
    int64_t buf[] = {1, -12511, 31415821, 37};
    for (int i = 0; i < 4; ++i) {
      CHECK(hash_buffer(buf + i, 1) == hash(buf[i]));
    }
  }

  SECTION("Collisions") {
    std::set<uint64_t> hashes;

    int collisions = 0;
    int same = 0;
    for (int i = 0; i < 10'000'000; ++i) {
      uint64_t h = hash(i);

      if (h == i) {
        ++same;
      }

      if (hashes.contains(h)) {
        ++collisions;
      } else {
        hashes.insert(h);
      }
    }

    CHECK(same == 0);
    CHECK(collisions == 0);
  }

  SECTION("Unaligned") {
    uint64_t buf[] = {0xfacebeef, 0x65028088, 0x13372048};
    char cbuf[sizeof(buf) + 8];
    for (int delta = 0; delta < 8; ++delta) {
      memcpy(cbuf + delta, buf, sizeof(buf));
      CHECK(hash_buffer(buf, 3) == hash_buffer(cbuf + delta, sizeof(buf)));
    }
  }
}
