#include "specula/util/rng.hpp"

#include <vector>

#include <catch2/catch_test_macros.hpp>

using namespace specula;

TEST_CASE("Rng", "[util]") {
  SECTION("Reseed") {
    Rng rng(1234);
    std::vector<uint32_t> values;
    values.reserve(100);
    for (int i = 0; i < 100; ++i) {
      values.push_back(rng.uniform<uint32_t>());
    }

    rng.set_sequence(1234);
    for (int i = 0; i < 100; ++i) {
      CHECK(values[i] == rng.uniform<uint32_t>());
    }
  }

  SECTION("Advance") {
    Rng rng;
    rng.set_sequence(1234, 6502);
    std::vector<float> v;
    v.reserve(1000);
    for (int i = 0; i < 1000; ++i) {
      v.push_back(rng.uniform<float>());
    }

    rng.set_sequence(1234, 6502);
    rng.advance(16);
    CHECK(rng.uniform<float>() == v[16]);

    for (int i = v.size() - 1; i >= 0; --i) {
      rng.set_sequence(1234, 6502);
      rng.advance(i);
      CHECK(rng.uniform<float>() == v[i]);
    }

    rng.set_sequence(32);
    rng.uniform<float>();

    for (int i : {5, 998, 552, 37, 16}) {
      rng.set_sequence(1234, 6502);
      rng.advance(i);
      CHECK(rng.uniform<float>() == v[i]);
    }
  }

  SECTION("Minus operator") {
    Rng ra(1337), rb(1337);
    Rng rng;

    for (int i = 0; i < 10; ++i) {
      int32_t step = rng.uniform<uint32_t>(1000) + 1;
      for (int j = 0; j < step; ++j) {
        ra.uniform<uint32_t>();
      }
      CHECK(step == ra - rb);
      CHECK(-step == rb - ra);

      if ((rng.uniform<uint32_t>() & 1) != 0u) {
        rb.advance(step);
      } else {
        ra.advance(-step);
      }

      CHECK(0 == ra - rb);
      CHECK(0 == rb - ra);
    }
  }

  SECTION("int") {
    Rng rng;
    int positive = 0, negative = 0, zero = 0;
    int count = 10'000;
    for (int i = 0; i < count; ++i) {
      int v = rng.uniform<int>();
      if (v < 0) {
        ++negative;
      } else if (v == 0) {
        ++zero;
      } else {
        ++positive;
      }
    }

    CHECK(positive > 0.48 * count);
    CHECK(positive < 0.52 * count);
    CHECK(negative > 0.48 * count);
    CHECK(negative < 0.52 * count);
    CHECK(zero < 0.001 * count);
  }

  SECTION("uint64") {
    Rng rng;
    std::array<int, 64> bit_counts = {0};
    int count = 10'000;
    for (int i = 0; i < count; ++i) {
      uint64_t v = rng.uniform<uint64_t>();
      for (int b = 0; b < 64; ++b) {
        if ((v & (1ull << b)) != 0u) {
          ++bit_counts[b];
        }
      }
    }

    for (int b = 0; b < 64; ++b) {
      CHECK(bit_counts[b] > 0.48 * count);
      CHECK(bit_counts[b] < 0.52 * count);
    }
  }

  SECTION("double") {
    Rng rng;
    for (int i = 0; i < 10; ++i) {
      double v = rng.uniform<double>();
      CHECK(v != float(v));
    }
  }
}
