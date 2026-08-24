#include "specula/util/math/functions.hpp"

#include <algorithm>
#include <vector>

int specula::next_prime(int x) {
  if (x == 2) {
    return 3;
  }
  if ((x & 1) == 0) {
    ++x;
  }

  std::vector<int> small_primes{2};
  auto is_prime = [&small_primes](int n) {
    return std::ranges::all_of(small_primes, [&n](int p) { return n == p || (n % p) != 0; });
  };

  const int max_prime_gap = 320;
  for (int n = 3; n < int(std::sqrt(x + max_prime_gap)) + 1; n += 2) {
    if (is_prime(n)) {
      small_primes.push_back(n);
    }
  }

  while (!is_prime(x)) {
    x += 2;
  }

  return x;
}
