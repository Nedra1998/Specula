#ifndef SPECULA_UTIL_MATH_FUNCTIONS_HPP
#define SPECULA_UTIL_MATH_FUNCTIONS_HPP

#include <cmath>
#include <cstdlib>
#include <limits>
#include <numbers>
#include <type_traits>

#include "specula/util/pstd/math.hpp"

#ifdef SPECULA_HAS_INTERN_H
#  include <intrin.h>
#endif

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math/constants.hpp"

namespace specula {

  SPECULA_CPU_GPU inline Float sin_x_over_x(Float x) {
    if (1 - x * x == 1) {
      return 1;
    }
    return std::sin(x) / x;
  }
  SPECULA_CPU_GPU inline Float lerp(Float x, Float a, Float b) { return (1 - x) * a + x * b; }

  template <typename T>
  SPECULA_CPU_GPU inline T fma(T a, T b, T c)
    requires(std::is_integral_v<T>)
  {
    return a * b + c;
  }

  SPECULA_CPU_GPU inline Float sinc(Float x) { return sin_x_over_x(PI * x); }
  SPECULA_CPU_GPU inline Float windowed_sinc(Float x, Float radius, Float tau) {
    if (std::abs(x) > radius) {
      return 0;
    }
    return sinc(x) * sinc(x / tau);
  }

  template <typename T, typename U, typename V>
  SPECULA_CPU_GPU inline constexpr T clamp(T val, U low, V high) {
    if (val < low) {
      return T(low);
    } else if (val > high) {
      return T(high);
    } else {
      return val;
    }
  }

  template <typename T> SPECULA_CPU_GPU inline T mod(T a, T b) {
    T result = a - (a / b) * b;
    return (T)((result < 0) ? result + b : result);
  }

  template <> SPECULA_CPU_GPU inline Float mod(Float a, Float b) { return std::fmod(a, b); }

  SPECULA_CPU_GPU inline Float radians(Float deg) { return (PI / 180) * deg; }
  SPECULA_CPU_GPU inline Float degrees(Float rad) { return (180 / PI) * rad; }

  SPECULA_CPU_GPU inline Float smooth_step(Float x, Float a, Float b) {
    if (a == b) {
      return (x < a) ? 0 : 1;
    }
    DASSERT_LT(a, b);
    Float t = clamp((x - a) / (b - a), 0, 1);
    return t * t * (3 - 2 * t);
  }

  SPECULA_CPU_GPU inline float safe_sqrt(float x) {
    DASSERT_GE(x, -1e-3f);
    return std::sqrt(std::max(0.f, x));
  }

  SPECULA_CPU_GPU inline double safe_sqrt(double x) {
    DASSERT_GE(x, -1e-3);
    return std::sqrt(std::max(0., x));
  }

  template <typename T> SPECULA_CPU_GPU inline constexpr T sqr(T v) { return v * v; }

  template <int N> SPECULA_CPU_GPU inline constexpr float pow(float v) {
    if constexpr (N < 0) {
      return 1 / pow<-N>(v);
    }
    float n2 = pow<N / 2>(v);
    return n2 * n2 * pow<N & 1>(v);
  }
  template <> SPECULA_CPU_GPU inline constexpr float pow<1>(float v) { return v; }
  template <> SPECULA_CPU_GPU inline constexpr float pow<0>(float v) { return 1; }

  template <int N> SPECULA_CPU_GPU inline constexpr double pow(double v) {
    if constexpr (N < 0) {
      return 1 / pow<-N>(v);
    }
    float n2 = pow<N / 2>(v);
    return n2 * n2 * pow<N & 1>(v);
  }
  template <> SPECULA_CPU_GPU inline constexpr double pow<1>(double v) { return v; }
  template <> SPECULA_CPU_GPU inline constexpr double pow<0>(double v) { return 1; }

  template <typename Float, typename C>
  SPECULA_CPU_GPU inline constexpr Float evaluate_polynomial(Float t, C c) {
    return c;
  }

  template <typename Float, typename C, typename... Args>
  SPECULA_CPU_GPU inline constexpr Float evaluate_polynomial(Float t, C c, Args... c_remaining) {
    return fma(t, evaluate_polynomial(t, c_remaining...), c);
  }

  SPECULA_CPU_GPU inline float safe_asin(float x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::asin(clamp(x, -1, 1));
  }
  SPECULA_CPU_GPU inline float safe_acos(float x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::acos(clamp(x, -1, 1));
  }

  SPECULA_CPU_GPU inline double safe_asin(double x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::asin(clamp(x, -1, 1));
  }
  SPECULA_CPU_GPU inline double safe_acos(double x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::acos(clamp(x, -1, 1));
  }

  SPECULA_CPU_GPU inline Float log2(Float x) {
    const Float invLog2 = std::numbers::log2e;
    return std::log(x) * invLog2;
  }

  SPECULA_CPU_GPU inline int log2_int(float v) {
    DASSERT_GE(v, 0);
    if (v < 1) {
      return -log2_int(1 / v);
    }

    const uint32_t midsignif = 0b00000000001101010000010011110011;
    return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
  }

  SPECULA_CPU_GPU inline int log2_int(double v) {
    DASSERT_GE(v, 0);
    if (v < 1) {
      return -log2_int(1 / v);
    }

    const uint64_t midsignif = 0b110101000001001111001100110011111110011101111001101;
    return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
  }

  SPECULA_CPU_GPU inline int log2_int(uint32_t v) {
#ifdef SPECULA_IS_GPU_CODE
    return 31 - __clz(v);
#elif defined(SPECULA_HAS_INTERN_H)
    unsigned long lz = 0;
    if (_BitScanReverse(&lz, v)) {
      return lz;
    }
    return 0;
#else
    return 31 - __builtin_clz(v);
#endif
  }

  SPECULA_CPU_GPU inline int log2_int(int32_t v) { return log2_int((uint32_t)v); }

  SPECULA_CPU_GPU inline int log2_int(uint64_t v) {
#ifdef SPECULA_IS_GPU_CODE
    return 63 - __clzll(v);
#elif defined(SPECULA_HAS_INTERN_H)
    unsigned long lz = 0;
#  ifdef _WIN64
    _BitScanReverse64(&lz, v);
#  else
    if (_BitScanReverse(&lz, v >> 32)) {
      lz += 32;
    } else {
      _BitScanReverse(&lz, v & 0xffffffff)
    }
#  endif
    return ls;
#else
    return 63 - __builtin_clzll(v);
#endif
  }

  SPECULA_CPU_GPU inline int log2_int(int64_t v) { return log2_int((uint64_t)v); }

  template <typename T> SPECULA_CPU_GPU inline int log4_int(T v) { return log2_int(v) / 2; }

  SPECULA_CPU_GPU inline float fast_exp(float x) {
#ifdef SPECULA_IS_GPU_CODE
    return __expf(x);
#else
    float xp = x * std::numbers::log2e_v<float>;
    float fxp = pstd::floor(xp);
    float f = xp - fxp;
    int i = (int)fxp;

    float two_to_f = evaluate_polynomial(f, 1.f, 0.695556856f, 0.226173572f, 0.0781455737f);

    int exponent = specula::exponent(two_to_f) + i;
    if (exponent < -126) {
      return 0;
    }
    if (exponent > 127) {
      return INFINITY;
    }

    uint32_t bits = float_to_bits(two_to_f);
    bits &= 0b10000000011111111111111111111111u;
    bits |= (exponent + 127) << 23;
    return bits_to_float(bits);

#endif
  }

  SPECULA_CPU_GPU inline Float gaussian(Float x, Float mu = 0, Float sigma = 1) {
    return 1 / std::sqrt(2 * PI * sigma * sigma) * fast_exp(-sqr(x - mu) / (2 * sigma * sigma));
  }

  SPECULA_CPU_GPU inline Float gaussian_integral(Float x0, Float x1, Float mu = 0,
                                                 Float sigma = 1) {
    DASSERT_GT(sigma, 0);
    Float sigma_root2 = sigma * std::numbers::sqrt2_v<Float>;
    return 0.5f * (std::erf((mu - x0) / sigma_root2) - std::erf((mu - x1) / sigma_root2));
  }

  SPECULA_CPU_GPU inline Float logistic(Float x, Float s) {
    x = std::abs(x);
    return std::exp(-x / s) / (s * sqr(1 + std::exp(-x / s)));
  }
  SPECULA_CPU_GPU inline Float logistic_cdf(Float x, Float s) { return 1 / (1 + std::exp(-x / s)); }

  SPECULA_CPU_GPU inline Float trimmed_logistic(Float x, Float s, Float a, Float b) {
    DASSERT_LT(a, b);
    return logistic(x, s) / (logistic_cdf(b, s) - logistic_cdf(a, s));
  }

  template <typename Predicate>
  SPECULA_CPU_GPU inline size_t find_interval(size_t sz, const Predicate &pred) {
    using ssize_t = std::make_signed_t<size_t>;
    ssize_t size = (ssize_t)sz - 2, first = 1;
    while (size > 0) {
      size_t half = (size_t)size >> 1, middle = first + half;
      bool pred_result = pred(middle);
      first = pred_result ? static_cast<ssize_t>(middle) + 1 : first;
      size = pred_result ? size - static_cast<ssize_t>(half + 1) : static_cast<ssize_t>(half);
    }
    return (size_t)clamp(first - 1, 0, sz - 2);
  }

  template <typename T> SPECULA_CPU_GPU inline constexpr bool is_power_of_2(T v) {
    return v && !(v & (v - 1));
  }

  template <typename T> SPECULA_CPU_GPU inline bool is_power_of_4(T v) {
    return v == 1 << (2 * log4_int(v));
  }

  SPECULA_CPU_GPU inline constexpr int32_t round_up_pow2(int32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v + 1;
  }
  SPECULA_CPU_GPU inline constexpr int64_t round_up_pow2(int64_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    return v + 1;
  }

  template <typename T> SPECULA_CPU_GPU inline T round_up_pow4(T v) {
    return is_power_of_4(v) ? v : (1 << (2 * (1 + log4_int(v))));
  }

  template <typename Ta, typename Tb, typename Tc, typename Td>
  SPECULA_CPU_GPU inline auto difference_of_products(Ta a, Tb b, Tc c, Td d) {
    auto cd = c * d;
    auto difference_of_products = fma(a, b, -cd);
    auto error = fma(-c, d, cd);
    return difference_of_products + error;
  }

  template <typename Ta, typename Tb, typename Tc, typename Td>
  SPECULA_CPU_GPU inline auto sum_of_products(Ta a, Tb b, Tc c, Td d) {
    auto cd = c * d;
    auto sum_of_products = fma(a, b, cd);
    auto error = fma(c, d, -cd);
    return sum_of_products + error;
  }

  SPECULA_CPU_GPU inline bool quadratic(float a, float b, float c, float *t0, float *t1) {
    if (a == 0) {
      if (b == 0) {
        return false;
      }
      *t0 = *t1 = -c / b;
      return true;
    }

    float discrim = difference_of_products(b, b, 4 * a, c);
    if (discrim < 0) {
      return false;
    }
    float root_discrim = std::sqrt(discrim);
    float q = -0.5f * (b + pstd::copysign(root_discrim, b));
    *t0 = q / a;
    *t1 = c / q;
    if (*t0 > *t1) {
      pstd::swap(*t0, *t1);
    }
    return true;
  }

  SPECULA_CPU_GPU inline bool quadratic(double a, double b, double c, double *t0, double *t1) {
    if (a == 0) {
      if (b == 0) {
        return false;
      }
      *t0 = *t1 = -c / b;
      return true;
    }

    double discrim = difference_of_products(b, b, 4 * a, c);
    if (discrim < 0) {
      return false;
    }
    double root_discrim = std::sqrt(discrim);
    double q = -0.5 * (b + pstd::copysign(root_discrim, b));
    *t0 = q / a;
    *t1 = c / q;
    if (*t0 > *t1) {
      pstd::swap(*t0, *t1);
    }
    return true;
  }

  template <typename Func>
  SPECULA_CPU_GPU inline Float newton_bisection(Float x0, Float x1, Func f, Float xeps = 1e-4f,
                                                Float feps = 1e-6f) {
    DASSERT_LT(x0, x1);
    Float fx0 = f(x0).first, fx1 = f(x1).first;
    if (std::abs(fx0) < feps) {
      return x0;
    }
    if (std::abs(fx1) < feps) {
      return x1;
    }
    bool start_is_negative = fx0 < 0;

    Float xmid = x0 + (x1 - x0) * -fx0 / (fx1 - fx0);

    while (true) {
      if (x0 >= xmid || xmid >= x1) {
        xmid = (x0 + x1) / 2;
      }

      std::pair<Float, Float> fxmid = f(xmid);
      DASSERT(!isnan(fxmid.first));
      if (start_is_negative == (fxmid.first < 0)) {
        x0 = xmid;
      } else {
        x1 = xmid;
      }

      if ((x1 - x0) < xeps || std::abs(fxmid.first) < feps) {
        return xmid;
      }
      xmid -= fxmid.first / fxmid.second;
    }
  }

  SPECULA_CPU_GPU inline Float erf_inv(Float a) {
#ifdef SPECULA_IS_GPU_CODE
    return erfinv(a);
#else
    float p = 0;
    float t = std::log(std::max(fma(a, -a, 1), std::numeric_limits<float>::min()));
    ASSERT(!isnan(t) && !isinf(t)); // NOLINT

    if (std::abs(t) > 6.125f) {
      p = 3.03697567e-10f;            //  0x1.4deb44p-32
      p = fma(p, t, 2.93243101e-8f);  //  0x1.f7c9aep-26
      p = fma(p, t, 1.22150334e-6f);  //  0x1.47e512p-20
      p = fma(p, t, 2.84108955e-5f);  //  0x1.dca7dep-16
      p = fma(p, t, 3.93552968e-4f);  //  0x1.9cab92p-12
      p = fma(p, t, 3.02698812e-3f);  //  0x1.8cc0dep-9
      p = fma(p, t, 4.83185798e-3f);  //  0x1.3ca920p-8
      p = fma(p, t, -2.64646143e-1f); // -0x1.0eff66p-2
      p = fma(p, t, 8.40016484e-1f);  //  0x1.ae16a4p-1
    } else {
      p = 5.43877832e-9f;             //  0x1.75c000p-28
      p = fma(p, t, 1.43286059e-7f);  //  0x1.33b458p-23
      p = fma(p, t, 1.22775396e-6f);  //  0x1.49929cp-20
      p = fma(p, t, 1.12962631e-7f);  //  0x1.e52bbap-24
      p = fma(p, t, -5.61531961e-5f); // -0x1.d70c12p-15
      p = fma(p, t, -1.47697705e-4f); // -0x1.35be9ap-13
      p = fma(p, t, 2.31468701e-3f);  //  0x1.2f6402p-9
      p = fma(p, t, 1.15392562e-2f);  //  0x1.7a1e4cp-7
      p = fma(p, t, -2.32015476e-1f); // -0x1.db2aeep-3
      p = fma(p, t, 8.86226892e-1f);  //  0x1.c5bf88p-1
    }

    return a * p;
#endif
  }

  SPECULA_CPU_GPU inline Float i0(Float x) {
    Float val = 0;
    Float x2i = 1;
    int64_t ifact = 1;
    int i4 = 1;

    for (int i = 0; i < 10; ++i) {
      if (i > 1) {
        ifact *= i;
      }
      val += x2i / static_cast<Float>(i4 * sqr(ifact));
      x2i *= x * x;
      i4 *= 4;
    }
    return val;
  }

  SPECULA_CPU_GPU inline Float log_i0(Float x) {
    if (x > 12) {
      return x + 0.5f * (-std::log(2 * PI) + std::log(1 / x) + 1 / (8 * x));
    } else {
      return std::log(i0(x));
    }
  }

  int next_prime(int x);

} // namespace specula

#endif // SPECULA_UTIL_MATH_FUNCTIONS_HPP
