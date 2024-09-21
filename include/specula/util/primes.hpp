/**
 * @file primes.hpp
 * @brief A table of 1000 precomputed prime numbers.
 */

#ifndef SPECULA_UTIL_PRIMES_HPP_
#define SPECULA_UTIL_PRIMES_HPP_

#include <cstddef>

#include <specula/specula.hpp>

namespace specula {
  /**
   * @brief The size of the precomputed prime table.
   */
  static constexpr std::size_t PRIME_TABLE_SIZE = 10000;

  /**
   * @brief A table of precomputed prime numbers.
   *
   * This is used for determining bases to use for the radical inverse base low-discrepancy points.
   * And it is precomputed to avoid the overhead of computing prime numbers at runtime.
   */
  extern SPECULA_CONST int PRIMES[PRIME_TABLE_SIZE];
} // namespace specula

#endif // SPECULA_UTIL_PRIMES_HPP_
