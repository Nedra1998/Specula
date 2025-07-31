#ifndef INCLUDE_MATH_FUNCTIONS_HPP_
#define INCLUDE_MATH_FUNCTIONS_HPP_

#include <type_traits>

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math/compensated.hpp"
#include "specula/util/math/constants.hpp"

namespace specula {

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

  inline SPECULA_CPU_GPU float safe_asin(float x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::asin(clamp(x, -1, 1));
  }
  inline SPECULA_CPU_GPU double safe_asin(double x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::asin(clamp(x, -1, 1));
  }

  inline SPECULA_CPU_GPU float safe_acos(float x) {
    DASSERT(x >= -1.0001 && x <= 1.0001);
    return std::acos(clamp(x, -1, 1));
  }

  inline SPECULA_CPU_GPU double safe_acos(double x) {
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

  SPECULA_CPU_GPU inline CompensatedFloat two_prod(Float a, Float b) {
    Float ab = a * b;
    return {ab, fma(a, b, -ab)};
  }

  SPECULA_CPU_GPU inline CompensatedFloat two_sum(Float a, Float b) {
    Float s = a + b, delta = s - a;
    return {s, (a - (s - delta)) + (b - delta)};
  }

  template <typename Ta, typename Tb, typename Tc, typename Td>
  SPECULA_CPU_GPU inline auto difference_of_products(Ta a, Tb b, Tc c, Td d) {
    auto cd = c * d;
    auto difference_of_products = fma(a, b, -cd);
    auto error = fma(-c, d, cd);
    return difference_of_products + error;
  }

  namespace detail {
    template <typename Float>
    SPECULA_CPU_GPU inline CompensatedFloat inner_product(Float a, Float b) {
      return two_prod(a, b);
    }

    template <typename Float, typename... Args>
    SPECULA_CPU_GPU inline CompensatedFloat inner_product(Float a, Float b, Args... terms) {
      CompensatedFloat ab = two_prod(a, b);
      CompensatedFloat tp = inner_product(terms...);
      CompensatedFloat sum = two_sum(ab.v, tp.v);
      return {sum.v, ab.err + (tp.err + sum.err)};
    }
  } // namespace detail

  template <typename... Args>
  SPECULA_CPU_GPU inline std::enable_if_t<std::conjunction_v<std::is_arithmetic<Args>...>, Float>
  inner_product(Args... terms) {
    CompensatedFloat ip = detail::inner_product(terms...);
    return Float(ip);
  }

} // namespace specula

#endif // INCLUDE_MATH_FUNCTIONS_HPP_
