#ifndef SPECULA_UTIL_FLOAT_HPP
#define SPECULA_UTIL_FLOAT_HPP

#include <limits>
#include <type_traits>

#include <fmt/format.h>

#include "specula/specula.hpp"
#include "specula/util/pstd.hpp"

namespace specula {

#ifdef SPECULA_IS_GPU_CODE

#  define DoubleOneMinusEpsilon 0x1.fffffffffffffp-1
#  define FloatOneMinusEpsilon float(0x1.fffffep-1)

#  ifdef SPECULA_FLOAT_AS_DOUBLE
#    define OneMinusEpsilon DoubleOneMinusEpsilon
#  else
#    define OneMinusEpsilon FloatOneMinusEpsilon
#  endif

#  define Infinity std::numeric_limits<Float>::infinity()
#  define MachineEpsilon std::numeric_limits<Float>::epsilon() * 0.5f

#else

  static constexpr Float Infinity = std::numeric_limits<Float>::infinity();
  static constexpr Float MachineEpsilon = std::numeric_limits<Float>::epsilon() * 0.5;

  static constexpr double DoubleOneMinusEpsilon = 0x1.fffffffffffffp-1;
  static constexpr float FloatOneMinusEpsilon = 0x1.fffffep-1;

#  ifdef SPECULA_FLOAT_AS_DOUBLE
  static constexpr Float OneMinusEpsilon = DoubleOneMinusEpsilon;
#  else
  static constexpr Float OneMinusEpsilon = FloatOneMinusEpsilon;
#  endif // SPECULA_FLOAT_AS_DOUBLE

#endif // SPECULA_IS_GPU_CODE

  static const int HalfExponentMask = 0b0111110000000000;
  static const int HalfSignificandMask = 0b1111111111;
  static const int HalfNegativeZero = 0b1000000000000000;
  static const int HalfPositiveZero = 0;
  // Exponent all 1s, significand zero
  static const int HalfNegativeInfinity = 0b1111110000000000;
  static const int HalfPositiveInfinity = 0b0111110000000000;

  template <typename T>
  inline SPECULA_CPU_GPU typename std::enable_if_t<std::is_floating_point_v<T>, bool> isnan(T v) {
#ifdef SPECULA_IS_GPU_CODE
    return isnan(v);
#else
    return std::isnan(v);
#endif // SPECULA_IS_GPU_CODE
  }

  template <typename T>
  inline SPECULA_CPU_GPU typename std::enable_if_t<std::is_integral_v<T>, bool> isnan(T v) {
    return false;
  }

  template <typename T>
  inline SPECULA_CPU_GPU typename std::enable_if_t<std::is_floating_point_v<T>, bool> isinf(T v) {
#ifdef SPECULA_IS_GPU_CODE
    return isinf(v);
#else
    return std::isinf(v);
#endif // SPECULA_IS_GPU_CODE
  }

  template <typename T>
  inline SPECULA_CPU_GPU typename std::enable_if_t<std::is_integral_v<T>, bool> isinf(T v) {
    return false;
  }

  template <typename T>
  inline SPECULA_CPU_GPU typename std::enable_if_t<std::is_floating_point_v<T>, bool>
  isfinite(T v) {
#ifdef SPECULA_IS_GPU_CODE
    return isfinite(v);
#else
    return std::isfinite(v);
#endif // SPECULA_IS_GPU_CODE
  }

  template <typename T>
  inline SPECULA_CPU_GPU typename std::enable_if_t<std::is_integral_v<T>, bool> isfinite(T v) {
    return true;
  }

  SPECULA_CPU_GPU inline float fma(float a, float b, float c) { return std::fma(a, b, c); }
  SPECULA_CPU_GPU inline double fma(double a, double b, double c) { return std::fma(a, b, c); }
  inline long double fma(long double a, long double b, long double c) { return std::fma(a, b, c); }

  SPECULA_CPU_GPU inline uint32_t float_to_bits(float f) {
#ifdef SPECULA_IS_GPU_CODE
    return __float_as_uint(f);
#else
    return pstd::bit_cast<uint32_t>(f);
#endif // SPECULA_IS_GPU_CODE
  }

  SPECULA_CPU_GPU inline uint64_t float_to_bits(double f) {
#ifdef SPECULA_IS_GPU_CODE
    return __double_as_longlong(f);
#else
    return pstd::bit_cast<uint64_t>(f);
#endif // SPECULA_IS_GPU_CODE
  }

  SPECULA_CPU_GPU inline float bits_to_float(uint32_t ui) {
#ifdef SPECULA_IS_GPU_CODE
    return __uint_as_float(ui);
#else
    return pstd::bit_cast<float>(ui);
#endif // SPECULA_IS_GPU_CODE
  }

  SPECULA_CPU_GPU inline double bits_to_float(uint64_t ui) {
#ifdef SPECULA_IS_GPU_CODE
    return _longlong_as_double(ui);
#else
    return pstd::bit_cast<double>(ui);
#endif // SPECULA_IS_GPU_CODE
  }

  SPECULA_CPU_GPU inline int exponent(float v) { return (float_to_bits(v) >> 23) - 127; }
  SPECULA_CPU_GPU inline int exponent(double v) { return (float_to_bits(v) >> 52) - 1023; }
  SPECULA_CPU_GPU inline int significand(float v) { return float_to_bits(v) & ((1 << 23) - 1); }
  SPECULA_CPU_GPU inline uint64_t significand(double v) {
    return float_to_bits(v) & ((1ull << 52) - 1);
  }
  SPECULA_CPU_GPU inline uint32_t sign_bit(float v) { return float_to_bits(v) & 0x80000000; }
  SPECULA_CPU_GPU inline uint64_t sign_bit(double v) {
    return float_to_bits(v) & 0x8000000000000000;
  }

  SPECULA_CPU_GPU inline float flip_sign(float a, float b) {
    return bits_to_float(float_to_bits(a) ^ sign_bit(b));
  }
  SPECULA_CPU_GPU inline double flip_sign(double a, double b) {
    return bits_to_float(float_to_bits(a) ^ sign_bit(b));
  }

  SPECULA_CPU_GPU inline float next_float_up(float v) {
    if (isinf(v) && v > 0.f)
      return v;
    if (v == -0.f)
      v = 0.f;
    uint32_t ui = float_to_bits(v);
    if (v >= 0)
      ++ui;
    else
      --ui;
    return bits_to_float(ui);
  }

  SPECULA_CPU_GPU inline double next_float_up(double v) {
    if (isinf(v) && v > 0.)
      return v;
    if (v == -0.)
      v = 0.;
    uint64_t ui = float_to_bits(v);
    if (v >= 0)
      ++ui;
    else
      --ui;
    return bits_to_float(ui);
  }

  SPECULA_CPU_GPU inline float next_float_down(float v) {
    if (isinf(v) && v < 0.f)
      return v;
    if (v == 0.f)
      v = -0.f;

    uint32_t ui = float_to_bits(v);
    if (v > 0)
      --ui;
    else
      ++ui;
    return bits_to_float(ui);
  }

  SPECULA_CPU_GPU inline double next_float_down(double v) {
    if (isinf(v) && v < 0.)
      return v;
    if (v == 0.)
      v = -0.;

    uint64_t ui = float_to_bits(v);
    if (v > 0)
      --ui;
    else
      ++ui;
    return bits_to_float(ui);
  }

  inline constexpr Float gamma(int n) { return (n * MachineEpsilon) / (1 - n * MachineEpsilon); }

  inline SPECULA_CPU_GPU Float add_round_up(Float a, Float b) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __dadd_ru(a, b);
#  else
    return __fadd_ru(a, b);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_up(a + b);
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float add_round_down(Float a, Float b) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __dadd_rd(a, b);
#  else
    return __fadd_rd(a, b);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_down(a + b);
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float sub_round_up(Float a, Float b) { return add_round_up(a, -b); }
  inline SPECULA_CPU_GPU Float sub_round_down(Float a, Float b) { return add_round_down(a, -b); }

  inline SPECULA_CPU_GPU Float mul_round_up(Float a, Float b) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __dmul_ru(a, b);
#  else
    return __fmul_ru(a, b);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_up(a * b);
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float mul_round_down(Float a, Float b) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __dmul_rd(a, b);
#  else
    return __fmul_rd(a, b);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_down(a * b);
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float div_round_up(Float a, Float b) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __ddiv_ru(a, b);
#  else
    return __fdiv_ru(a, b);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_up(a / b);
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float div_round_down(Float a, Float b) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __ddiv_rd(a, b);
#  else
    return __fdiv_rd(a, b);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_down(a / b);
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float sqrt_round_up(Float a) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __dsqrt_ru(a);
#  else
    return __fsqrt_ru(a);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_up(std::sqrt(a));
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float sqrt_round_down(Float a) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __dsqrt_rd(a);
#  else
    return __fsqrt_rd(a);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return std::max<Float>(0, next_float_down(std::sqrt(a)));
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float fma_round_up(Float a, Float b, Float c) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __fma_ru(a, b, c); // FIXME: What to do here?
#  else
    return __fma_ru(a, b, c);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_up(fma(a, b, c));
#endif // SPECULA_IS_GPU_CODE
  }

  inline SPECULA_CPU_GPU Float fma_round_down(Float a, Float b, Float c) {
#ifdef SPECULA_IS_GPU_CODE
#  ifdef SPECULA_FLOAT_AS_DOUBLE
    return __fma_rd(a); // FIXME: What to do here?
#  else
    return __fma_rd(a);
#  endif // SPECULA_FLOAT_AS_DOUBLE
#else
    return next_float_down(fma(a, b, c));
#endif // SPECULA_IS_GPU_CODE
  }

  namespace {
    // TODO: Support for non-AVX systems, check CPUID, etc...

    union FP32 {
      uint32_t u;
      float f;
      struct {
        unsigned int mantissa : 23;
        unsigned int exponent : 8;
        unsigned int sign : 1;
      };
    };

    union FP16 {
      uint16_t u;
      struct {
        unsigned int mantissa : 10;
        unsigned int exponent : 5;
        unsigned int sign : 1;
      };
    };
  } // namespace

  class Half {
  public:
    Half() = default;
    Half(const Half &) = default;
    Half &operator=(const Half &) = default;

    SPECULA_CPU_GPU static Half from_bits(uint16_t v) { return Half(v); }

    SPECULA_CPU_GPU explicit Half(float ff) {
#ifdef SPECULA_IS_GPU_CODE
      h = __half_as_ushort(__float2half(ff));
#else

      FP32 f;
      f.f = ff;
      FP32 f32infty = {255 << 23};
      FP32 f16max = {(127 + 16) << 23};
      FP32 denorm_magic = {((127 - 15) + (23 - 10) + 1) << 23};
      unsigned int sign_mask = 0x80000000u;
      FP16 o = {0};

      unsigned int sign = f.u & sign_mask;
      f.u ^= sign;

      if (f.u >= f16max.u)
        o.u = (f.u > f32infty.u) ? 0x7e00 : 0x7c00;
      else {
        if (f.u < (113 << 23)) {
          f.f += denorm_magic.f;
          o.u = f.u - denorm_magic.u;
        } else {
          unsigned int mant_odd = (f.u >> 13) & 1;
          f.u += (uint32_t(15 - 127) << 23) + 0xfff;
          f.u += mant_odd;
          o.u = f.u >> 13;
        }
      }
      o.u |= sign >> 16;
      h = o.u;
#endif // SPECULA_IS_GPU_CODE
    }

    SPECULA_CPU_GPU explicit Half(double d) : Half(float(d)) {}

    SPECULA_CPU_GPU explicit operator float() const {
#ifdef SPECULA_IS_GPU_CODE
      return __half2float(__ushort_as_half(h));
#else

      FP16 h;
      h.u = this->h;
      static const FP32 magic = {113 << 23};
      static const unsigned int shifted_exp = 0x7c00 << 13;

      FP32 o;
      o.u = (h.u & 0x7fff) << 13;
      unsigned int exp = shifted_exp & o.u;
      o.u += (127 - 15) << 23;

      if (exp == shifted_exp)
        o.u += (128 - 16) << 23;
      else if (exp == 0) {
        o.u += 1 << 23;
        o.f -= magic.f;
      }

      o.u |= (h.u & 0x8000) << 16;
      return o.f;
#endif // SPECULA_IS_GPU_CODE
    }

    SPECULA_CPU_GPU explicit operator double() const { return (float)(*this); }

    SPECULA_CPU_GPU bool operator==(const Half &v) const {
#if defined(SPECULA_IS_GPU_CODE) && __CUDA_ARCH__ >= 540
      return __ushort_as_half(h) == __ushort_as_half(v.h);
#else
      if (bits() == v.bits())
        return true;
      return ((bits() == HalfNegativeZero && v.bits() == HalfPositiveZero) ||
              (bits() == HalfPositiveZero && v.bits() == HalfNegativeZero));
#endif
    }

    SPECULA_CPU_GPU bool operator!=(const Half &v) const { return !(*this == v); }

    SPECULA_CPU_GPU Half operator-() const { return from_bits(h ^ (1 << 15)); }

    SPECULA_CPU_GPU uint16_t bits() const { return h; }

    SPECULA_CPU_GPU int sign() { return (h >> 15) ? -1 : 1; }

    SPECULA_CPU_GPU bool isinf() { return h == HalfPositiveInfinity || h == HalfNegativeInfinity; }

    SPECULA_CPU_GPU bool isnan() {
      return ((h & HalfExponentMask) == HalfExponentMask && (h & HalfSignificandMask) != 0);
    }

    SPECULA_CPU_GPU Half next_up() {
      if (isinf() && sign() == 1)
        return *this;

      Half up = *this;
      if (up.h == HalfNegativeZero)
        up.h = HalfPositiveZero;
      if (up.sign() >= 0)
        ++up.h;
      else
        --up.h;
      return up;
    }

    SPECULA_CPU_GPU Half next_down() {
      if (isinf() && sign() == -1)
        return *this;

      Half down = *this;
      if (down.h == HalfPositiveZero)
        down.h = HalfNegativeZero;
      if (down.sign() >= 0)
        --down.h;
      else
        ++down.h;
      return down;
    }

  private:
    SPECULA_CPU_GPU explicit Half(uint16_t h) : h(h) {}

    uint16_t h;
  };

  inline auto format_as(Half h) { return (float)h; }
  inline std::ostream &operator<<(std::ostream &os, Half h) { return os << fmt::format("{}", h); }

} // namespace specula

#endif // !SPECULA_UTIL_FLOAT_HPP
