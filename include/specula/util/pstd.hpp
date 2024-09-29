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

#include <cmath>
#include <type_traits>

#include "specula/specula.hpp"

// IWYU pragma: begin_exports
#include "specula/util/pstd/array.hpp"
#include "specula/util/pstd/complex.hpp"
#include "specula/util/pstd/optional.hpp"
#include "specula/util/pstd/pmr.hpp"
#include "specula/util/pstd/span.hpp"
#include "specula/util/pstd/tuple.hpp"
#include "specula/util/pstd/vector.hpp"
// IWYU pragma: end_exports

/**
 * @brief Standard library functions available on both the host and device
 *
 *  This namespace contains a set of functions that are available on both the host and device. Such
 * that they can be used in both CPU and GPU code.
 */
namespace specula::pstd {

  template <typename T> SPECULA_CPU_GPU inline void swap(T &a, T &b) {
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
  }

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

  SPECULA_CPU_GPU inline float sqrt(float f) { return ::sqrtf(f); }
  SPECULA_CPU_GPU inline double sqrt(double f) { return ::sqrt(f); }
  SPECULA_CPU_GPU inline float abs(float f) { return ::fabsf(f); }
  SPECULA_CPU_GPU inline double abs(double f) { return ::fabs(f); }

  SPECULA_CPU_GPU inline float copysign(float mag, float sign) {
#ifdef SPECULA_IS_GPU_CODE
    return ::copysignf(mag, sign);
#else
    return std::copysign(mag, sign);
#endif
  }

  SPECULA_CPU_GPU inline double copysign(double mag, double sign) {
#ifdef SPECULA_IS_GPU_CODE
    return ::copysignf(mag, sign);
#else
    return std::copysign(mag, sign);
#endif
  }

  SPECULA_CPU_GPU inline float floor(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::floorf(arg);
#else
    return std::floor(arg);
#endif
  }

  SPECULA_CPU_GPU inline double floor(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::floor(arg);
#else
    return std::floor(arg);
#endif
  }

  SPECULA_CPU_GPU inline float ceil(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::ceilf(arg);
#else
    return std::ceil(arg);
#endif
  }

  SPECULA_CPU_GPU inline double ceil(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::ceil(arg);
#else
    return std::ceil(arg);
#endif
  }

  SPECULA_CPU_GPU inline float round(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::roundf(arg);
#else
    return std::round(arg);
#endif
  }

  SPECULA_CPU_GPU inline double round(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::roundf(arg);
#else
    return std::round(arg);
#endif
  }

  SPECULA_CPU_GPU inline float fmod(float x, float y) {
#ifdef SPECULA_IS_GPU_CODE
    return ::fmodf(x, y);
#else
    return std::fmod(x, y);
#endif
  }

  SPECULA_CPU_GPU inline double fmod(double x, double y) {
#ifdef SPECULA_IS_GPU_CODE
    return ::fmod(x, y);
#else
    return std::fmod(x, y);
#endif
  }

} // namespace specula::pstd

#endif // !SPECULA_UTIL_PSTD_HPP
