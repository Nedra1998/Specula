/**
 * @file
 * @brief GPU-safe `float`/`double` overloads of a few `<cmath>` functions
 *
 * The `std::` math functions are not callable from device code, so each wrapper forwards to the
 * C/CUDA global function (`::floorf`, ...) under `SPECULA_IS_GPU_CODE` and to `std::` otherwise.
 * Only `float` and `double` overloads exist: integer arguments are ambiguous and `long double`
 * is unsupported. Semantics are exactly those of the `std` function of the same name.
 *
 * @see https://en.cppreference.com/w/cpp/header/cmath
 */

#ifndef SPECULA_UTIL_PSTD_MATH_HPP
#define SPECULA_UTIL_PSTD_MATH_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cmath>

#include "specula/macros.hpp"

namespace specula::pstd {
  /**
   * @brief Square root. @see https://en.cppreference.com/w/cpp/numeric/math/sqrt
   * @todo Unlike the other wrappers, `sqrt` and `abs` always call the global C function, even on
   * the host; `<cmath>` does not guarantee that `::sqrtf`/`::fabsf` are declared.
   */
  SPECULA_CPU_GPU inline float sqrt(float f) { return ::sqrtf(f); }
  /** @brief Square root. */
  SPECULA_CPU_GPU inline double sqrt(double f) { return ::sqrt(f); }
  /** @brief Absolute value. @see https://en.cppreference.com/w/cpp/numeric/math/fabs */
  SPECULA_CPU_GPU inline float abs(float f) { return ::fabsf(f); }
  /** @brief Absolute value. */
  SPECULA_CPU_GPU inline double abs(double f) { return ::fabs(f); }

  /**
   * @brief Magnitude of `mag` with the sign bit of `sign` (honours signed zero and NaN sign).
   * @see https://en.cppreference.com/w/cpp/numeric/math/copysign
   */
  SPECULA_CPU_GPU inline float copysign(float mag, float sign) {
#ifdef SPECULA_IS_GPU_CODE
    return ::copysignf(mag, sign);
#else
    return std::copysign(mag, sign);
#endif
  }
  /** @brief Magnitude of `mag` with the sign bit of `sign`. */
  SPECULA_CPU_GPU inline double copysign(double mag, double sign) {
#ifdef SPECULA_IS_GPU_CODE
    return ::copysign(mag, sign);
#else
    return std::copysign(mag, sign);
#endif
  }

  /** @brief Round toward @f$ -\infty @f$. @see https://en.cppreference.com/w/cpp/numeric/math/floor
   */
  SPECULA_CPU_GPU inline float floor(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::floorf(arg);
#else
    return std::floor(arg);
#endif
  }
  /** @brief Round toward @f$ -\infty @f$. */
  SPECULA_CPU_GPU inline double floor(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::floor(arg);
#else
    return std::floor(arg);
#endif
  }

  /** @brief Round toward @f$ +\infty @f$. @see https://en.cppreference.com/w/cpp/numeric/math/ceil
   */
  SPECULA_CPU_GPU inline float ceil(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::ceilf(arg);
#else
    return std::ceil(arg);
#endif
  }
  /** @brief Round toward @f$ +\infty @f$. */
  SPECULA_CPU_GPU inline double ceil(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::ceil(arg);
#else
    return std::ceil(arg);
#endif
  }

  /**
   * @brief Round to nearest, halfway cases away from zero (not round-half-to-even).
   * @see https://en.cppreference.com/w/cpp/numeric/math/round
   */
  SPECULA_CPU_GPU inline float round(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::roundf(arg);
#else
    return std::round(arg);
#endif
  }
  /** @brief Round to nearest, halfway cases away from zero. */
  SPECULA_CPU_GPU inline double round(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::round(arg);
#else
    return std::round(arg);
#endif
  }

  /**
   * @brief Truncated remainder of `x / y`; the result has the sign of `x`, so this is not a
   * mathematical modulo for negative `x`.
   * @see https://en.cppreference.com/w/cpp/numeric/math/fmod
   */
  SPECULA_CPU_GPU inline float fmod(float x, float y) {
#ifdef SPECULA_IS_GPU_CODE
    return ::fmodf(x, y);
#else
    return std::fmod(x, y);
#endif
  }
  /** @brief Truncated remainder of `x / y`, with the sign of `x`. */
  SPECULA_CPU_GPU inline double fmod(double x, double y) {
#ifdef SPECULA_IS_GPU_CODE
    return ::fmod(x, y);
#else
    return std::fmod(x, y);
#endif
  }
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_MATH_HPP
