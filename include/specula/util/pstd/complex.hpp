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

  template <typename T> struct complex {
    SPECULA_CPU_GPU complex(T re) : re(re), im(0) {}
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
