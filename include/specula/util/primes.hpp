/**
 * @file
 * @brief A table of 10,000 precomputed prime numbers.
 */

#ifndef SPECULA_UTIL_PRIMES_HPP
#define SPECULA_UTIL_PRIMES_HPP

#include <cstddef>

namespace specula {
  /// The size of the prime table, which contains the first 10,000 prime numbers.
  static constexpr std::size_t PRIME_TABLE_SIZE = 10000;

  /**
   * @brief A table of the first 10,000 prime numbers.
   *
   * A table of precomputed prime numbers, which can be used for various
   * purposes within the renderer.
   */
  extern const int PRIMES[PRIME_TABLE_SIZE];
} // namespace specula

#endif // SPECULA_UTIL_PRIMES_HPP
