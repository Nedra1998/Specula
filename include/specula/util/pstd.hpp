/**
 * @file pstd.hpp
 * @brief Utilities for portable standard library
 *
 * This file provides a set of utilities for working with the portable standard, which is a subset
 * of the standard library that is available on both the host and device, to allow the code to be
 * compiled for both CPU and GPU platforms.
 */

#ifndef SPECULA_UTIL_PSTD_HPP
#define SPECULA_UTIL_PSTD_HPP

#include <type_traits>

#include "specula/specula.hpp"

/**
 * @brief Standard library functions available on both the host and device
 *
 *  This namespace contains a set of functions that are available on both the host and device. Such
 * that they can be used in both CPU and GPU code.
 */
namespace specula::pstd {

  /**
   * @brief Bit-cast between types
   *
   * This function provides a bit-cast to convert between types by mearly copying the bits from one
   * type to another.
   *
   * @tparam To Destination type
   * @tparam From Source type
   * @param src Source value
   * @return constexpr To
   */
  template <class To, class From>
  SPECULA_CPU_GPU
      typename std::enable_if_t<sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&
                                    std::is_trivially_copyable_v<To>,
                                To>
      bit_cast(const From &src) noexcept {
    static_assert(
        std::is_trivially_constructible_v<To>,
        "This implementation requires the destination type to be trivially constructable");
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
  }

} // namespace specula::pstd

#endif // !SPECULA_UTIL_PSTD_HPP
