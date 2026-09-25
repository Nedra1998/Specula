#include "specula/util/low_discrepancy.hpp"

#include "specula/util/primes.hpp"

specula::pstd::vector<specula::DigitPermuation> *
specula::compute_radical_inverse_permutations(uint32_t seed, Allocator alloc) {
  auto *perms = alloc.new_object<pstd::vector<DigitPermuation>>(alloc);
  perms->resize(PRIME_TABLE_SIZE);
  for (size_t i = 0; i < PRIME_TABLE_SIZE; ++i) {
    (*perms)[i] = DigitPermuation(PRIMES[i], seed, alloc);
  }
  return perms;
}
