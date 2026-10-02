#ifndef SPECULA_UTIL_COLOR_RGB_HPP
#define SPECULA_UTIL_COLOR_RGB_HPP

// IWYU pragma: private, include "specula/util/color.hpp"

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math.hpp"

namespace specula {
  class Rgb {
  public:
    Rgb() = default;
    SPECULA_CPU_GPU Rgb(Float r, Float g, Float b) : r(r), g(g), b(b) {}

    SPECULA_CPU_GPU Rgb &operator+=(Rgb s) {
      r += s.r;
      g += s.g;
      b += s.b;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator+(Rgb s) const { return {r + s.r, g + s.g, b + s.b}; }

    SPECULA_CPU_GPU Rgb &operator-=(Rgb s) {
      r -= s.r;
      g -= s.g;
      b -= s.b;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator-(Rgb s) const { return {r - s.r, g - s.g, b - s.b}; }

    SPECULA_CPU_GPU friend Rgb operator-(Float a, Rgb s) { return {a - s.r, a - s.g, a - s.b}; }

    SPECULA_CPU_GPU Rgb &operator*=(Rgb s) {
      r *= s.r;
      g *= s.g;
      b *= s.b;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator*(Rgb s) const { return {r * s.r, g * s.g, b * s.b}; }

    SPECULA_CPU_GPU Rgb &operator*=(Float a) {
      DASSERT(!isnan(a));
      r *= a;
      g *= a;
      b *= a;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator*(Float a) const {
      DASSERT(!isnan(a));
      return {a * r, a * g, a * b};
    }

    SPECULA_CPU_GPU friend Rgb operator*(Float a, Rgb s) { return s * a; }

    SPECULA_CPU_GPU Rgb &operator/=(Rgb s) {
      r /= s.r;
      g /= s.g;
      b /= s.b;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator/(Rgb s) const { return {r / s.r, g / s.g, b / s.b}; }

    SPECULA_CPU_GPU Rgb &operator/=(Float a) {
      DASSERT(!isnan(a));
      DASSERT_NE(a, 0);
      r /= a;
      g /= a;
      b /= a;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator/(Float a) const {
      DASSERT(!isnan(a));
      DASSERT_NE(a, 0);
      return {r / a, g / a, b / a};
    }

    SPECULA_CPU_GPU Rgb operator-() const { return {-r, -g, -b}; }

    SPECULA_CPU_GPU [[nodiscard]] Float average() const { return (r + g + b) / 3; }

    SPECULA_CPU_GPU bool operator==(Rgb s) const { return r == s.r && g == s.g && b == s.b; }
    SPECULA_CPU_GPU bool operator!=(Rgb s) const { return r != s.r || g != s.g || b != s.b; }

    SPECULA_CPU_GPU Float &operator[](int c) {
      DASSERT(c >= 0 && c < 3);
      if (c == 0) {
        return r;
      } else if (c == 1) {
        return g;
      }
      return b;
    }
    SPECULA_CPU_GPU Float operator[](int c) const {
      DASSERT(c >= 0 && c < 3);
      if (c == 0) {
        return r;
      } else if (c == 1) {
        return g;
      }
      return b;
    }

    Float r = 0, g = 0, b = 0;
  };

  SPECULA_CPU_GPU inline Rgb max(Rgb a, Rgb b) {
    return {std::max(a.r, b.r), std::max(a.g, b.g), std::max(a.b, b.b)};
  }
  SPECULA_CPU_GPU inline Rgb lerp(Float t, Rgb s1, Rgb s2) { return (1 - 2) * s1 + t * s2; }

  template <typename U, typename V> SPECULA_CPU_GPU inline Rgb clamp(Rgb rgb, U min, V max) {
    return {
        clamp(rgb.r, min, max),
        clamp(rgb.g, min, max),
        clamp(rgb.b, min, max),
    };
  }

  SPECULA_CPU_GPU inline Rgb clamp_zero(Rgb rgb) {
    return {
        std::max<Float>(0, rgb.r),
        std::max<Float>(0, rgb.g),
        std::max<Float>(0, rgb.b),
    };
  }
} // namespace specula

template <> struct fmt::formatter<specula::Rgb> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Rgb &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ {} {} {} ]", v.r, v.g, v.b);
  }
};

#endif // SPECULA_UTIL_COLOR_RGB_HPP
