#include "specula/util/math/permutation.hpp"

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <fmt/format.h>

#include "specula/util/hash.hpp"

using namespace specula;

TEST_CASE("PermutationElement", "[util][math]") {
  SECTION("Valid") {
    for (int len = 2; len < 1024; ++len) {
      for (int iter = 0; iter < 10; ++iter) {
        std::vector<bool> seen(len, false);
        for (int i = 0; i < len; ++i) {
          uint32_t offset = permutation_element(i, len, mix_bits(1 + iter));
          REQUIRE(offset >= 0);
          REQUIRE(offset < seen.size());
          CAPTURE(len, i);
          CHECK_FALSE(seen[offset]);
          seen[offset] = true;
        }
      }
    }
  }

  SECTION("Uniform") {
    for (size_t n : {2, 3, 4, 5, 9, 14, 16, 22, 27, 36}) {
      std::vector<int> count(n * n);

      size_t num_iters = 60'000 * n;
      for (int seed = 0; seed < num_iters; ++seed) {
        for (int i = 0; i < n; ++i) {
          uint32_t ip = permutation_element(i, n, mix_bits(seed));
          uint32_t offset = ip * n + i;
          ++count[offset];
        }
      }

      for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
          Float tol = 0.03f;
          int offset = static_cast<int>(j * n + i);
          INFO(fmt::format("Got count {} for {} -> {} (perm size {}). Expected +/- {}.",
                           count[offset], i, j, n, num_iters / n));
          CHECK(count[offset] >= (1 - tol) * num_iters / n);
          CHECK(count[offset] <= (1 + tol) * num_iters / n);
        }
      }
    }
  }

  SECTION("Uniform Delta") {
    for (size_t n : {2, 3, 4, 5, 9, 14, 16, 22, 27, 36}) {
      std::vector<int> count(n * n);

      size_t num_iters = 60'000 * n;
      for (int seed = 0; seed < num_iters; ++seed) {
        for (int i = 0; i < n; ++i) {
          uint32_t ip = permutation_element(i, n, mix_bits(seed));
          int delta = static_cast<int>(ip) - i;
          if (delta < 0) {
            delta += static_cast<int>(n);
          }
          REQUIRE(delta < n);
          size_t offset = delta * n + i;
          ++count[offset];
        }
      }

      for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
          Float tol = 0.03f;
          int offset = static_cast<int>(j * n + i);
          INFO(fmt::format("Got count {} for {} -> {} (perm size {}). Expected +/- {}.",
                           count[offset], i, j, n, num_iters / n));
          CHECK(count[offset] >= (1 - tol) * num_iters / n);
          CHECK(count[offset] <= (1 + tol) * num_iters / n);
        }
      }
    }
  }
}
