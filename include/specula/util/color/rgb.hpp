#ifndef INCLUDE_COLOR_RGB_HPP_
#define INCLUDE_COLOR_RGB_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"

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

    SPECULA_CPU_GPU Rgb *operator-=(Rgb s) {
      r -= s.r;
      g -= s.g;
      b -= s.b;
      return this;
    }

    SPECULA_CPU_GPU Rgb operator-(Rgb s) const { return {r - s.r, g - s.g, b - s.b}; }

    SPECULA_CPU_GPU friend Rgb operator-(Float a, Rgb b) { return {a - b.r, a - b.g, a - b.b}; }

    SPECULA_CPU_GPU Rgb operator*=(Rgb s) {
      r *= s.r;
      g *= s.g;
      b *= s.b;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator*(Rgb s) const { return {r * s.r, g * s.g, b * s.b}; }

    SPECULA_CPU_GPU Rgb operator*=(Float s) {
      r *= s;
      g *= s;
      b *= s;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator*(Float s) const { return {r * s, g * s, b * s}; }

    SPECULA_CPU_GPU friend Rgb operator*(Float s, Rgb c) { return {s * c.r, s * c.g, s * c.b}; }

    SPECULA_CPU_GPU Rgb &operator/=(Rgb s) {
      DASSERT(!isnan(s.r) && !isnan(s.g) && !isnan(s.b));
      DASSERT_NE(s.r, 0.0f);
      DASSERT_NE(s.g, 0.0f);
      DASSERT_NE(s.b, 0.0f);
      r /= s.r;
      g /= s.g;
      b /= s.b;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator/(Rgb s) const {
      DASSERT(!isnan(s.r) && !isnan(s.g) && !isnan(s.b));
      DASSERT_NE(s.r, 0.0f);
      DASSERT_NE(s.g, 0.0f);
      DASSERT_NE(s.b, 0.0f);
      return {r / s.r, g / s.g, b / s.b};
    }

    SPECULA_CPU_GPU Rgb &operator/=(Float s) {
      DASSERT(!isnan(s));
      DASSERT_NE(s, 0.0f);
      r /= s;
      g /= s;
      b /= s;
      return *this;
    }

    SPECULA_CPU_GPU Rgb operator/(Float s) const {
      DASSERT(!isnan(s));
      DASSERT_NE(s, 0.0f);
      return {r / s, g / s, b / s};
    }

    SPECULA_CPU_GPU Rgb operator-() const { return {-r, -g, -b}; }

    SPECULA_CPU_GPU Float average() const { return (r + g + b) / 3.0f; }

    SPECULA_CPU_GPU bool operator==(Rgb s) const { return r == s.r && g == s.g && b == s.b; }
    SPECULA_CPU_GPU bool operator!=(Rgb s) const { return r != s.r || g != s.g || b != s.b; }

    SPECULA_CPU_GPU Float operator[](size_t i) const {
      DASSERT(i >= 0 && i <= 2);
      if (i == 0)
        return r;
      else if (i == 1)
        return g;
      else
        return b;
    }

    SPECULA_CPU_GPU Float &operator[](size_t i) {
      DASSERT(i >= 0 && i <= 2);
      if (i == 0)
        return r;
      else if (i == 1)
        return g;
      else
        return b;
    }

    Float r = 0.0, g = 0.0, b = 0.0;
  };

  SPECULA_CPU_GPU inline Rgb max(Rgb a, Rgb b) {
    return {std::max(a.r, b.r), std::max(a.g, b.g), std::max(a.b, b.b)};
  }

  SPECULA_CPU_GPU inline Rgb lerp(Float t, Rgb t0, Rgb t1) { return (1 - t) * t0 + t * t1; }

  template <typename U, typename V> SPECULA_CPU_GPU inline Rgb clamp(Rgb rgb, U min, V max) {
    return Rgb(clamp(rgb.r, min, max), clamp(rgb.g, min, max), clamp(rgb.b, min, max));
  }

  template <typename U, typename V> SPECULA_CPU_GPU inline Rgb clamp_zero(Rgb rgb) {
    return Rgb(std::max<Float>(0, rgb.r), std::max<Float>(0, rgb.g), std::max<Float>(0, rgb.b));
  }

  inline auto format_as(Rgb c) { return std::array<Float, 3>{c.r, c.g, c.b}; }

  inline std::ostream &operator<<(std::ostream &os, const Rgb &c) {
    return os << fmt::format("{}", c);
  }

} // namespace specula

#endif // INCLUDE_COLOR_RGB_HPP_
