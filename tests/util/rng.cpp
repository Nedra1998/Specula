#include <array>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <specula/util/rng.hpp>

using namespace specula;

TEST_CASE("RNG", "[util]") {
  SECTION("Reseed") {
    Rng rng(1234);
    std::vector<std::uint32_t> initial_values;
    for (std::size_t i = 0; i < 100; ++i)
      initial_values.push_back(rng.uniform<std::uint32_t>());

    rng.set_sequence(1234);
    std::vector<std::uint32_t> reseeded_values;
    for (std::size_t i = 0; i < 100; ++i)
      reseeded_values.push_back(rng.uniform<std::uint32_t>());

    CHECK_THAT(reseeded_values, Catch::Matchers::Equals(initial_values));
  }

  SECTION("Advance") {
    Rng rng(1234, 6502);
    std::vector<float> v;
    for (std::size_t i = 0; i < 1000; ++i)
      v.push_back(rng.uniform<float>());

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

  SECTION("Operator Minus") {
    Rng ra(1337), rb(1337);
    Rng rng;
    for (std::size_t i = 0; i < 10; ++i) {
      int step = 1 + rng.uniform<std::uint32_t>(1000);
      for (std::size_t j = 0; j < step; ++j) {
        (void)ra.uniform<std::uint32_t>();
      }

      CHECK(ra - rb == step);
      CHECK(rb - ra == -step);

      if ((rng.uniform<std::uint32_t>() & 1) != 0u)
        rb.advance(step);
      else
        ra.advance(-step);

      CHECK(ra - rb == 0);
      CHECK(rb - ra == 0);
    }
  }

  SECTION("Integer") {
    Rng rng;
    std::size_t positive = 0, negative = 0, zero = 0, count = 10000;
    for (std::size_t i = 0; i < count; ++i) {
      int v = rng.uniform<int>();
      if (v < 0)
        ++negative;
      else if (v == 0)
        ++zero;
      else
        ++positive;
    }

    CHECK(positive > 0.48 * count);
    CHECK(positive < 0.52 * count);
    CHECK(negative > 0.48 * count);
    CHECK(negative < 0.52 * count);
    CHECK(zero < 0.001 * count);
  }

  SECTION("Unsigned 64") {
    Rng rng;
    std::array<std::size_t, 64> bit_counts = {0};
    std::size_t count = 10000;
    for (std::size_t i = 0; i < count; ++i) {
      std::uint64_t v = rng.uniform<std::uint64_t>();
      for (std::uint8_t b = 0; b < 64; ++b) {
        if ((v & (1ull << b)) != 0u)
          ++bit_counts[b];
      }
    }

    for (std::uint8_t b = 0; b < 64; ++b) {
      CHECK(bit_counts[b] > 0.48 * count);
      CHECK(bit_counts[b] < 0.52 * count);
    }
  }
}
