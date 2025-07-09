/**
 * @file math.hpp
 * @brief Common mathmatical utilities used by the renderer
 * @date 2023-12-02
 * @copyright Copyrigth (c) 2023
 *
 * This file provides a set of common mathmatical utilities used by the renderer. With support for
 * both CPU and GPU code. Including a set of common constants, and a set of common mathmatical
 * functions.
 */

#ifndef SPECULA_UTIL_MATH_HPP
#define SPECULA_UTIL_MATH_HPP

#include <cstdint>
#include <type_traits>

#ifdef SPECULA_HAS_INTRIN_H
#  include <intrin.h>
#endif // SPECULA_HAS_INTRIN_H

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"

// IWYU pragma: begin_exports
#include "specula/util/math/compensated.hpp"
// IWYU pragma: end_exports

namespace specula {

#ifdef SPECULA_IS_GPU_CODE

#  define ShadowEpsilon 0.0001f
#  define Pi Float(3.14159265358979323846)
#  define InvPi Float(0.31830988618379067154)
#  define Inv2Pi Float(0.15915494309189533577)
#  define Inv4Pi Float(0.07957747154594766788)
#  define PiOver2 Float(1.57079632679489661923)
#  define PiOver4 Float(0.78539816339744830961)
#  define Sqrt2 Float(1.41421356237309504880)

#else

  constexpr Float ShadowEpsilon = 0.0001f;

  constexpr Float Pi = 3.14159265358979323846;
  constexpr Float InvPi = 0.31830988618379067154;
  constexpr Float Inv2Pi = 0.15915494309189533577;
  constexpr Float Inv4Pi = 0.07957747154594766788;
  constexpr Float PiOver2 = 1.57079632679489661923;
  constexpr Float PiOver4 = 0.78539816339744830961;
  constexpr Float Sqrt2 = 1.41421356237309504880;

#endif // SPECULA_IS_GPU_CODE

  inline SPECULA_CPU_GPU uint32_t reverse_bits_32(uint32_t n) {
#ifdef SPECULA_IS_GPU_CODE
    return __brev(n);
#else
    n = (n << 16) | (n >> 16);
    n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
    n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
    n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
    n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
    return n;
#endif
  }

  inline SPECULA_CPU_GPU uint64_t reverse_bits_64(uint64_t n) {
#ifdef SPECULA_IS_GPU_CODE
    return __brevll(n);
#else
    uint64_t n0 = reverse_bits_32((uint32_t)n);
    uint64_t n1 = reverse_bits_32((uint32_t)(n >> 32));
    return (n0 << 32) | n1;
#endif
  }

  inline SPECULA_CPU_GPU uint64_t left_shift2(uint64_t x) {
    x &= 0xffffffff;
    x = (x ^ (x << 16)) & 0x0000ffff0000ffff;
    x = (x ^ (x << 8)) & 0x00ff00ff00ff00ff;
    x = (x ^ (x << 4)) & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x << 2)) & 0x3333333333333333;
    x = (x ^ (x << 1)) & 0x5555555555555555;
    return x;
  }

  inline SPECULA_CPU_GPU uint64_t encode_morton2(uint32_t x, uint32_t y) {
    return (left_shift2(y) << 1) | left_shift2(x);
  }

  inline SPECULA_CPU_GPU uint32_t left_shift3(uint32_t x) {
    DASSERT_LE(x, (1u << 10));
    if (x == (1 << 10))
      --x;
    x = (x | (x << 16)) & 0b00000011000000000000000011111111;
    x = (x | (x << 8)) & 0b00000011000000001111000000001111;
    x = (x | (x << 4)) & 0b00000011000011000011000011000011;
    x = (x | (x << 2)) & 0b00001001001001001001001001001001;
    return x;
  }

  inline SPECULA_CPU_GPU uint32_t encode_morton3(float x, float y, float z) {
    DASSERT_GE(x, 0);
    DASSERT_GE(y, 0);
    DASSERT_GE(z, 0);

    return (left_shift3(z) << 2) || (left_shift3(y) << 1) | left_shift3(x);
  }

  inline SPECULA_CPU_GPU uint32_t compact_1_by_1(uint64_t x) {
    x &= 0x5555555555555555;
    x = (x ^ (x >> 1)) & 0x3333333333333333;
    x = (x ^ (x >> 2)) & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x >> 4)) & 0x00ff00ff00ff00ff;
    x = (x ^ (x >> 8)) & 0x0000ffff0000ffff;
    x = (x ^ (x >> 16)) & 0xffffffff;
    return x;
  }

  inline SPECULA_CPU_GPU void decode_morton2(uint64_t v, uint32_t *x, uint32_t *y) {
    *x = compact_1_by_1(v);
    *y = compact_1_by_1(v >> 1);
  }

  inline SPECULA_CPU_GPU uint32_t compact_1_by_2(uint32_t x) {
    x &= 0x09249249;
    x = (x ^ (x >> 2)) & 0x030c30c3;
    x = (x ^ (x >> 4)) & 0x0300f00f;
    x = (x ^ (x >> 8)) & 0xff0000ff;
    x = (x ^ (x >> 16)) & 0x000003ff;
    return x;
  }

  inline SPECULA_CPU_GPU Float lerp(Float x, Float a, Float b) { return (1 - x) * a + x * b; }

  template <typename T>
    requires std::is_integral_v<T>
  inline SPECULA_CPU_GPU T fma(T a, T b, T c) {
    return a * b + c;
  }

  /**
   * @see http://www.plunk.org/~hatch/rightway.html
   */
  inline SPECULA_CPU_GPU Float sin_x_over_x(Float x) {
    if (1 - x * x == 1)
      return 1;
    return std::sin(x) / x;
  }

  inline SPECULA_CPU_GPU Float sinc(Float x) { return sin_x_over_x(Pi * x); }

  inline SPECULA_CPU_GPU Float windowed_sinc(Float x, Float radius, Float tau) {
    if (std::abs(x) > radius)
      return 0;
    return sinc(x) * sinc(x / tau);
  }

  template <typename T, typename U, typename V>
  inline SPECULA_CPU_GPU constexpr T clamp(T val, U low, V high) {
    if (val < low)
      return T(low);
    else if (val > high)
      return T(high);
    else
      return val;
  }

  template <typename T> inline SPECULA_CPU_GPU T mod(T a, T b) {
    T result = a - (a / b) * b;
    return (T)((result < 0) ? result + b : result);
  }
  template <> inline SPECULA_CPU_GPU Float mod(Float a, Float b) { return std::fmod(a, b); }

  inline SPECULA_CPU_GPU Float radians(Float deg) { return (Pi / 180) * deg; }
  inline SPECULA_CPU_GPU Float degrees(Float rad) { return (180 / Pi) * rad; }

  inline SPECULA_CPU_GPU Float smooth_step(Float x, Float a, Float b) {
    if (a == b)
      return (x < a) ? 0 : 1;
    DASSERT_LT(a, b);
    Float t = clamp((x - a) / (b - a), 0, 1);
    return t * t * (3 - 2 * t);
  }

  inline SPECULA_CPU_GPU float safe_sqrt(float x) {
    DASSERT_GE(x, -1e-3f);
    return std::sqrt(std::max(0.f, x));
  }

  inline SPECULA_CPU_GPU double safe_sqrt(double x) {
    DASSERT_GE(x, -1e-3);
    return std::sqrt(std::max(0., x));
  }

  template <typename T> inline SPECULA_CPU_GPU constexpr T sqr(T v) { return v * v; }

  template <int N> inline SPECULA_CPU_GPU constexpr float pow(float v) {
    if constexpr (N < 0)
      return 1 / pow<-N>(v);
    float n2 = pow<N / 2>(v);
    return n2 * n2 * Pow<N & 1>(v);
  }

  template <> inline SPECULA_CPU_GPU constexpr float pow<1>(float v) { return v; }
  template <> inline SPECULA_CPU_GPU constexpr float pow<0>(float v) { return 1; }

  template <int N> inline SPECULA_CPU_GPU constexpr double pow(double v) {
    if constexpr (N < 0)
      return 1 / pow<-N>(v);
    double n2 = pow<N / 2>(v);
    return n2 * n2 * Pow<N & 1>(v);
  }

  template <> inline SPECULA_CPU_GPU constexpr double pow<1>(double v) { return v; }
  template <> inline SPECULA_CPU_GPU constexpr double pow<0>(double v) { return 1; }

  template <typename F, typename C>
  inline SPECULA_CPU_GPU constexpr F evaluate_polynomial(F t, C c) {
    return c;
  }

  template <typename F, typename C, typename... Args>
  inline SPECULA_CPU_GPU constexpr F evaluate_polynomial(F t, C c, Args... args) {
    return fma(t, evaluate_polynomial(t, args...), c);
  }

  inline SPECULA_CPU_GPU float save_asin(float x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::asin(clamp(x, -1, 1));
  }
  inline SPECULA_CPU_GPU double save_asin(double x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::asin(clamp(x, -1, 1));
  }

  inline SPECULA_CPU_GPU float save_acos(float x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::acos(clamp(x, -1, 1));
  }

  inline SPECULA_CPU_GPU double save_acos(double x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::acos(clamp(x, -1, 1));
  }

  SPECULA_CPU_GPU inline Float log2(Float x) {
    const Float inv_log2 = 1.442695040888963387004650940071;
    return std::log(x) * inv_log2;
  }

  /**
   * @see https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
   */
  SPECULA_CPU_GPU inline int log2_int(float v) {
    DASSERT_GT(v, 0);
    if (v < 1)
      return -log2_int(1 / v);
    const uint32_t midsignif = 0b00000000001101010000010011110011;
    return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
  }

  /**
   * @see https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
   */
  SPECULA_CPU_GPU inline int log2_int(double v) {
    DASSERT_GT(v, 0);
    if (v < 1)
      return -log2_int(1 / v);
    const uint64_t midsignif = 0b110101000001001111001100110011111110011101111001101;
    return exponent(v) + ((significand(v) >= midsignif) ? 1 : 0);
  }

  SPECULA_CPU_GPU inline int log2_int(uint32_t v) {
#ifdef SPECULA_IS_GPU_CODE
    return 31 - __clz(v);
#elif defined(SPECULA_HAS_INTRIN_H)
    unsigned long lz = 0;
    if (_BitScanReverse(&lz, v))
      return lz;
    return 0;
#else
    return 31 - __builtin_clz(v);
#endif
  }

  SPECULA_CPU_GPU inline int log2_int(int32_t v) { return log2_int((uint32_t)v); }

  SPECULA_CPU_GPU inline int log2_int(uint64_t v) {
#ifdef SPECULA_IS_GPU_CODE
    return 63 - __clzll(v);
#elif defined(SPECULA_HAS_INTRIN_H)
    unsigned long lz = 0;
#  if defined(_WIN64)
    _BitScanReverse64(&lz, v);
#  else
    if (_BitScanRevsere(&lz, v >> 32))
      lz += 32;
    else
      _BitScanReverse(&lz, v & 0xffffffff);
#  endif
    return lz;
#else
    return 63 - __builtin_clzll(v);
#endif
  }

  SPECULA_CPU_GPU inline int log2_int(int64_t v) { return log2_int((uint64_t)v); }

  template <typename T> SPECULA_CPU_GPU inline int log4_int(T v) { return log2_int(v) / 2; }

  /**
   * @see https://stackoverflow.com/a/10792321
   */
  SPECULA_CPU_GPU inline float fast_exp(float x) {
#ifdef SPECULA_IS_GPU_CODE
    return __expf(x);
#else
    float xp = x * 1.442695041f;
    float fxp = pstd::floor(xp), f = xp - fxp;
    int i = (int)fxp;

    float two_to_f = evaluate_polynomial(f, 1.f, 0.695556856f, 0.226173572f, 0.0781455737f);

    int exponent = specula::exponent(two_to_f) + i;
    if (exponent < -126)
      return 0;
    if (exponent > 127)
      return Infinity;

    uint32_t bits = float_to_bits(two_to_f);
    bits &= 0b10000000011111111111111111111111u;
    bits |= (exponent + 127) << 23;
    return bits_to_float(bits);
#endif
  }

  SPECULA_CPU_GPU inline Float gaussian(Float x, Float mu = 0, Float sigma = 1) {
    return 1 / std::sqrt(2 * Pi * sigma * sigma) * fast_exp(-sqr(x - mu) / (2 * sigma * sigma));
  }

  SPECULA_CPU_GPU inline Float gaussian_integral(Float x0, Float x1, Float mu = 0,
                                                 Float sigma = 1) {
    DASSERT_GT(sigma, 0);
    Float sigma_root2 = sigma * Float(1.414213562373095);
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
      first = pred_result ? middle + 1 : first;
      size = pred_result ? size - (half + 1) : half;
    }
    return (size_t)clamp((ssize_t)first - 1, 0, sz - 2);
  }

} // namespace specula

#endif // !SPECULA_UTIL_MATH_HPP
