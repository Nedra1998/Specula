/**
 * @file complex.hpp
 * @brief Portable complex number implementation
 *
 * This file provides a portable complex number implementation that is compatible
 * with both the CPU and GPU. The implementation is based on the C++ standard
 * library, but it is extended to support GPU execution.
 */

#ifndef INCLUDE_PSTD_COMPLEX_HPP_
#define INCLUDE_PSTD_COMPLEX_HPP_

#include "specula/specula.hpp"

namespace specula::pstd {
  SPECULA_CPU_GPU inline float sqrt(float f);
  SPECULA_CPU_GPU inline double sqrt(double f);
  SPECULA_CPU_GPU inline float abs(float f);
  SPECULA_CPU_GPU inline double abs(double f);
  SPECULA_CPU_GPU inline float copysign(float mag, float sign);
  SPECULA_CPU_GPU inline double copysign(double mag, double sign);

  /**
   * @class complex
   * @brief A complex number type
   *
   * @tparam T The type of the real and imaginary parts
   * @see https://en.cppreference.com/w/cpp/numeric/complex
   */
  template <typename T> struct complex {
    /// @brief Construct a complex number with the imaginary part set to zero
    SPECULA_CPU_GPU complex(T re) : re(re), im(0) {}
    /// @brief Construct a complex number with the given real and imaginary parts
    SPECULA_CPU_GPU complex(T re, T im) : re(re), im(im) {}

    SPECULA_CPU_GPU complex operator-() const { return complex(-re, -im); }

    SPECULA_CPU_GPU complex operator+(const complex &z) const {
      return complex(re + z.re, im + z.im);
    }
    SPECULA_CPU_GPU complex operator-(const complex &z) const {
      return complex(re - z.re, im - z.im);
    }

    SPECULA_CPU_GPU complex operator*(const complex &z) const {
      return complex(re * z.re - im * z.im, re * z.im + im * z.re);
    }
    SPECULA_CPU_GPU complex operator/(const complex &z) const {
      T scale = 1 / (z.re * z.re + z.im * z.im);
      return complex(scale * (re * z.re + im * z.im), scale * (im * z.re - re * z.im));
    }

    friend SPECULA_CPU_GPU complex operator+(T value, complex z) { return complex(value) + z; }
    friend SPECULA_CPU_GPU complex operator-(T value, complex z) { return complex(value) - z; }
    friend SPECULA_CPU_GPU complex operator*(T value, complex z) { return complex(value) * z; }
    friend SPECULA_CPU_GPU complex operator/(T value, complex z) { return complex(value) / z; }

    /// @brief The real part of the complex number
    T re;
    /// @brief The imaginary part of the complex number
    T im;
  };

  /// @brief Returns the real part of the complex number
  template <typename T> SPECULA_CPU_GPU T real(const complex<T> &z) { return z.re; }
  /// @brief Returns the imaginary part of the complex number
  template <typename T> SPECULA_CPU_GPU T imag(const complex<T> &z) { return z.im; }
  /// @brief Returns the squared magnitude of the complex number
  template <typename T> SPECULA_CPU_GPU T norm(const complex<T> &z) {
    return z.re * z.re + z.im * z.im;
  }
  /// @brief Returns the magnitude of the complex number
  template <typename T> SPECULA_CPU_GPU T abs(const complex<T> &z) {
    return pstd::sqrt(pstd::norm(z));
  }

  /// @brief Complex square root in the range of the right half-plane
  template <typename T> SPECULA_CPU_GPU complex<T> sqrt(const complex<T> &z) {
    T n = pstd::abs(z), t1 = pstd::sqrt(T(.5) * (n + pstd::abs(z.re))), t2 = T(.5) * z.im / t1;

    if (n == 0)
      return 0;
    else if (z.re >= 0)
      return {t1, t2};
    else
      return {pstd::abs(t2), pstd::copysign(t1, z.im)};
  }
} // namespace specula::pstd

#endif // INCLUDE_PSTD_COMPLEX_HPP_
