#ifndef SPECULA_UTIL_PSTD_COMPLEX_HPP
#define SPECULA_UTIL_PSTD_COMPLEX_HPP

#include "specula/macros.hpp"
#include "specula/util/pstd/math.hpp"

namespace specula::pstd {
  template <typename T> struct complex {
    SPECULA_CPU_GPU complex(T re) : re(re), im(0) {}
    SPECULA_CPU_GPU complex(T re, T im) : re(re), im(im) {}

    SPECULA_CPU_GPU complex operator-() const { return {-re, -im}; }

    SPECULA_CPU_GPU complex operator+(complex z) const { return {re + z.re, im + z.im}; }
    SPECULA_CPU_GPU complex operator-(complex z) const { return {re - z.re, im - z.im}; }
    SPECULA_CPU_GPU complex operator*(complex z) const {
      return {re * z.re - im * z.im, re * z.im + im * z.re};
    }
    SPECULA_CPU_GPU complex operator/(complex z) const {
      T scale = 1.0 / (z.re * z.re + z.im * z.im);
      return {scale * (re * z.re + im * z.im), scale * (im * z.re - re * z.im)};
    }

    friend SPECULA_CPU_GPU complex operator+(T value, complex z) { return complex(value) + z; }
    friend SPECULA_CPU_GPU complex operator-(T value, complex z) { return complex(value) - z; }
    friend SPECULA_CPU_GPU complex operator*(T value, complex z) { return complex(value) * z; }
    friend SPECULA_CPU_GPU complex operator/(T value, complex z) { return complex(value) / z; }

    T re, im;
  };

  template <typename T> SPECULA_CPU_GPU T real(const complex<T> &z) { return z.re; }
  template <typename T> SPECULA_CPU_GPU T imag(const complex<T> &z) { return z.im; }
  template <typename T> SPECULA_CPU_GPU T norm(const complex<T> &z) {
    return z.re * z.re + z.im * z.im;
  }
  template <typename T> SPECULA_CPU_GPU T abs(const complex<T> &z) {
    return pstd::sqrt(pstd::norm(z));
  }
  template <typename T> SPECULA_CPU_GPU complex<T> sqrt(const complex<T> &z) {
    T n = pstd::abs(z);
    T t1 = pstd::sqrt(T(0.5) * (n + pstd::abs(z.re)));
    T t2 = T(0.5) * z.im / t1;

    if (n == 0) {
      return 0;
    }
    if (z.re >= 0) {
      return {t1, t2};
    }
    return {pstd::abs(t2), pstd::copysign(t1, z.im)};
  }
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_COMPLEX_HPP
