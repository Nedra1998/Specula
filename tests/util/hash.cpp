#include <set>

#include <catch2/catch_test_macros.hpp>
#include <specula/util/hash.hpp>

using namespace specula;

TEST_CASE("MurmurHash64A", "[util]") {
  SECTION("Empty string") {
    CHECK(murmur_hash_64A((const std::uint8_t *)"", 0, 0x00000000) == 0x0000000000000000);
    CHECK(murmur_hash_64A((const std::uint8_t *)"", 0, 0x00000001) == 0xc6a4a7935bd064dc);
    CHECK(murmur_hash_64A((const std::uint8_t *)"", 0, 0xffffffff) == 0x952d4201a42f3c31);
  }

  SECTION("Test string") {
    CHECK(murmur_hash_64A((const std::uint8_t *)"test", 4, 0x00000000) == 0x2f4a8724618f4c63);
    CHECK(murmur_hash_64A((const std::uint8_t *)"test", 4, 0x9747b28c) == 0xeb01435bbd4da813);
  }
}

TEST_CASE("Hash", "[util]") {
  SECTION("VarArgs") {
    std::int64_t buf[] = {1, -12511, 31415821, 37};
    for (std::size_t i = 0; i < 4; ++i) {
      CHECK(hash_buffer(buf + i, sizeof(std::int64_t)) == hash(buf[i]));
    }
  }

  SECTION("Collisions") {
    std::set<std::uint32_t> low, high;
    std::set<std::uint64_t> full;

    std::size_t low_collisions = 0, high_collisions = 0, full_collisions = 0;
    std::size_t same = 0;

    for (std::size_t i = 0; i < 10000000; ++i) {
      std::uint64_t h = hash(i);
      if (h == i)
        ++same;

      if (low.find(h & 0xffffffff) != low.end())
        ++low_collisions;
      if (high.find((h >> 32) & 0xffffffff) != high.end())
        ++high_collisions;
      if (full.find(h) != full.end())
        ++full_collisions;
    }

    CHECK(same == 0);
    CHECK(low_collisions == 0);
    CHECK(high_collisions == 0);
    CHECK(full_collisions == 0);
  }

  SECTION("Unaligned") {
    std::uint64_t buf[] = {0xfacebeef, 0x65028088, 0x13372048};
    std::uint8_t cbuf[sizeof(buf) + 8];

    for (int delta = 0; delta < 8; ++delta) {
      std::memcpy(cbuf + delta, buf, sizeof(buf));
      CHECK(hash_buffer(buf, sizeof(buf)) == hash_buffer(cbuf + delta, sizeof(buf)));
    }
  }
}
