/**
 * @file
 * @brief GPU-safe replacements for `std::swap` and `std::bit_cast`
 *
 * Neither `std::swap` (not `__device__`) nor `std::bit_cast` (C++20, not available in all device
 * toolchains) can be called from GPU code, so pbrt provides these minimal versions.
 */

#ifndef SPECULA_UTIL_PSTD_UTILITY_HPP
#define SPECULA_UTIL_PSTD_UTILITY_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cstring>
#include <type_traits>
#include <utility>

#include "specula/macros.hpp"

namespace specula::pstd {
  /**
   * @brief Exchanges `a` and `b` via three moves.
   *
   * @note Unlike `std::swap` this is unconditionally `noexcept`, not `constexpr`, unconstrained,
   * and has no array overload.
   * @warning Being an unconstrained template in `pstd`, an unqualified `swap(a, b)` that finds
   * both this and `std::swap` through ADL (e.g. on a `pstd` type wrapping a `std` type) is
   * ambiguous; call it qualified.
   * @see https://en.cppreference.com/w/cpp/algorithm/swap
   */
  template <typename T> SPECULA_CPU_GPU inline void swap(T &a, T &b) noexcept {
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
  }

  /**
   * @brief Reinterprets the object representation of `src` as a `To`.
   *
   * Implemented with `memcpy` into a default-initialised `To`, which is why `To` must also be
   * trivially constructible (`std::bit_cast` has no such requirement).
   *
   * @note Not `constexpr`, unlike `std::bit_cast`.
   * @see https://en.cppreference.com/w/cpp/numeric/bit_cast
   */
  template <class To, class From>
  SPECULA_CPU_GPU To bit_cast(const From &src) noexcept
    requires(sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&
             std::is_trivially_copyable_v<To> && std::is_trivially_constructible_v<To>)
  {
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
  }
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_UTILITY_HPP
