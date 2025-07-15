/**
 * @file rgb.hpp
 * @brief RGB color class definition and operations
 *
 * This file defines the `Rgb` class, which represents a color in the RGB color space.
 * It includes various operations such as addition, subtraction, multiplication, division,
 * clamping, and interpolation.
 */

#ifndef INCLUDE_COLOR_RGB_HPP_
#define INCLUDE_COLOR_RGB_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"

namespace specula {

  /**
   * @class Rgb
   * @brief A class representing a color in the RGB color space.
   *
   * The `Rgb` class encapsulates a color defined by its red, green, and blue components.
   * It provides various operations for manipulating colors, such as addition, subtraction,
   * multiplication, division, and clamping.
   */
  class Rgb {
  public:
    /// @brief Default constructor initializes RGB components to zero (black).
    Rgb() = default;

    /// @brief Constructor that initializes RGB components to specified values.
    SPECULA_CPU_GPU Rgb(Float r, Float g, Float b) : r(r), g(g), b(b) {}

    /**
     * @brief Computes the average of the RGB components.
     *
     * @return The average value of the red, green, and blue components.
     */
    SPECULA_CPU_GPU Float average() const { return (r + g + b) / 3.0f; }

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

  /**
   * @brief Computes the maximum of two RGB colors component-wise.
   *
   * @param a The first RGB color.
   * @param b The second RGB color.
   * @return The RGB color with each component being the maximum of the corresponding components of
   * `a` and `b`.
   */
  SPECULA_CPU_GPU inline Rgb max(Rgb a, Rgb b) {
    return {std::max(a.r, b.r), std::max(a.g, b.g), std::max(a.b, b.b)};
  }

  /**
   * @brief Linear interpolation between two RGB colors.
   *
   * @param t The interpolation factor, where 0 corresponds to `t0` and 1 corresponds to `t1`.
   * @param t0 The start RGB color.
   * @param t1 The end RGB color.
   * @return A new RGB color that is the result of linear interpolation between `t0` and `t1`.
   */
  SPECULA_CPU_GPU inline Rgb lerp(Float t, Rgb t0, Rgb t1) { return (1 - t) * t0 + t * t1; }

  /**
   * @brief Clamps the RGB color components to a specified range.
   *
   * @tparam U Value type for the minimum bound.
   * @tparam V Value type for the maximum bound.
   * @param rgb The RGB color to clamp.
   * @param min The minimum value for each component.
   * @param max The maximum value for each component.
   * @return A new RGB color with each component clamped to the specified range.
   */
  template <typename U, typename V> SPECULA_CPU_GPU inline Rgb clamp(Rgb rgb, U min, V max) {
    return Rgb(clamp(rgb.r, min, max), clamp(rgb.g, min, max), clamp(rgb.b, min, max));
  }

  /**
   * @brief Clamps the RGB color components to a minimum of zero.
   *
   * @param rgb The RGB color to clamp.
   * @return A new RGB color with each component clamped to a minimum of zero.
   */
  SPECULA_CPU_GPU inline Rgb clamp_zero(Rgb rgb) {
    return Rgb(std::max<Float>(0, rgb.r), std::max<Float>(0, rgb.g), std::max<Float>(0, rgb.b));
  }
} // namespace specula

template <> struct fmt::formatter<specula::Rgb> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Rgb &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ {} {} {} ]", v.r, v.g, v.b);
  }
};

#endif // INCLUDE_COLOR_RGB_HPP_
