/**
 * @file xyz.hpp
 * @brief XYZ color class definition and operations
 *
 * This file defines the `Xyz` class, which represents a color in the CIE XYZ
 * color space. It includes various operations such as addition, subtraction,
 * multiplication, division, clamping, and interpolation.
 */

#ifndef INCLUDE_COLOR_XYZ_HPP_
#define INCLUDE_COLOR_XYZ_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {

  /**
   * @class Xyz
   * @brief A class representing a color in the CIE XYZ color space.
   *
   * The `Xyz` class encapsulates a color defined by its X, Y, and Z
   * components. It provides various operations for manipulating colors, such
   * as addition, subtraction, multiplication, division, and clamping.
   *
   * XYZ color space is a linear color space that serves as a device-independent
   * model for color representation. It is based on human vision and is used as
   * a reference for other color spaces. The components X, Y, and Z correspond
   * to the human perception of color and are related to the biological response
   * of the rods and cones in the human eye.
   */
  class Xyz {
  public:
    /// @brief Default constructor initializes XYZ components to zero.
    Xyz() = default;

    /// @brief Constructor that initializes XYZ components to specified values.
    SPECULA_CPU_GPU Xyz(Float x, Float y, Float z) : x(x), y(y), z(z) {}

    /**
     * @brief Creates an `Xyz` color from chromaticity coordinates and luminance.
     *
     * @param xy The chromaticity coordinates (x, y).
     * @param Y The luminance, defaults to 1.
     * @return The corresponding `Xyz` color.
     */
    SPECULA_CPU_GPU static Xyz from_xyY(Point2f xy, Float Y = 1) {
      if (xy.y == 0)
        return Xyz(0.0, 0.0, 0.0);
      return Xyz(xy.x * Y / xy.y, Y, (1 - xy.x - xy.y) * Y / xy.y);
    }

    /**
     * @brief Computes the average of the XYZ components.
     *
     * @return The average value of the X, Y, and Z components.
     */
    SPECULA_CPU_GPU Float average() const { return (x + y + z) / 3.0f; }

    /**
     * @brief Computes the chromaticity coordinates (x, y) from the XYZ color.
     *
     * @return A `Point2f` containing the (x, y) chromaticity coordinates.
     */
    SPECULA_CPU_GPU Point2f xy() const { return Point2f(x / (x + y + z), y / (x + y + z)); }

    SPECULA_CPU_GPU Xyz &operator+=(Xyz s) {
      x += s.x;
      y += s.y;
      z += s.z;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator+(Xyz s) const { return {x + s.x, y + s.y, z + s.z}; }

    SPECULA_CPU_GPU Xyz &operator-=(Xyz s) {
      x -= s.x;
      y -= s.y;
      z -= s.z;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator-(Xyz s) const { return {x - s.x, y - s.y, z - s.z}; }

    SPECULA_CPU_GPU friend Xyz operator-(Float a, const Xyz &s) {
      return {a - s.x, a - s.y, a - s.z};
    }

    SPECULA_CPU_GPU Xyz &operator*=(Xyz s) {
      x *= s.x;
      y *= s.y;
      z *= s.z;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator*(Xyz s) const { return {x * s.x, y * s.y, z * s.z}; }

    SPECULA_CPU_GPU Xyz &operator*=(Float a) {
      DASSERT(!isnan(a));
      x *= a;
      y *= a;
      z *= a;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator*(Float a) const {
      DASSERT(!isnan(a));
      return {x * a, y * a, z * a};
    }

    SPECULA_CPU_GPU friend Xyz operator*(Float a, const Xyz &c) {
      DASSERT(!isnan(a));
      return {a * c.x, a * c.y, a * c.z};
    }

    SPECULA_CPU_GPU Xyz &operator/=(Xyz s) {
      DASSERT(!isnan(s.x) && !isnan(s.y) && !isnan(s.z));
      DASSERT_NE(s.x, 0.0f);
      DASSERT_NE(s.y, 0.0f);
      DASSERT_NE(s.z, 0.0f);
      x /= s.x;
      y /= s.y;
      z /= s.z;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator/(Xyz s) const {
      DASSERT(!isnan(s.x) && !isnan(s.y) && !isnan(s.z));
      DASSERT_NE(s.x, 0.0f);
      DASSERT_NE(s.y, 0.0f);
      DASSERT_NE(s.z, 0.0f);
      return {x / s.x, y / s.y, z / s.z};
    }

    SPECULA_CPU_GPU Xyz &operator/=(Float a) {
      DASSERT(!isnan(a));
      DASSERT_NE(a, 0.0f);
      x /= a;
      y /= a;
      z /= a;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator/(Float a) const {
      DASSERT(!isnan(a));
      DASSERT_NE(a, 0.0f);
      return {x / a, y / a, z / a};
    }

    SPECULA_CPU_GPU Xyz operator-() const { return {-x, -y, -z}; }

    SPECULA_CPU_GPU bool operator==(Xyz s) const { return x == s.x && y == s.y && z == s.z; }

    SPECULA_CPU_GPU bool operator!=(Xyz s) const { return x != s.x || y != s.y || z != s.z; }

    SPECULA_CPU_GPU Float operator[](size_t i) const {
      DASSERT(i >= 0 && i < 3);
      if (i == 0)
        return x;
      else if (i == 1)
        return y;
      else
        return z;
    }

    SPECULA_CPU_GPU Float &operator[](size_t i) {
      DASSERT(i >= 0 && i < 3);
      if (i == 0)
        return x;
      else if (i == 1)
        return y;
      else
        return z;
    }

    Float x = 0.0f, y = 0.0f, z = 0.0f;
  };

  /**
   * @brief Clamps the XYZ color components to a specified range.
   *
   * @tparam U Value type for the minimum bound.
   * @tparam V Value type for the maximum bound.
   * @param xyz The XYZ color to clamp.
   * @param min The minimum value for each component.
   * @param max The maximum value for each component.
   * @return A new XYZ color with each component clamped to the specified range.
   */
  template <typename U, typename V> SPECULA_CPU_GPU inline Xyz clamp(Xyz xyz, U min, V max) {
    return Xyz(clamp(xyz.x, min, max), clamp(xyz.y, min, max), clamp(xyz.z, min, max));
  }

  /**
   * @brief Clamps the XYZ color components to a minimum of zero.
   *
   * @param xyz The XYZ color to clamp.
   * @return A new XYZ color with each component clamped to a minimum of zero.
   */
  template <typename U, typename V> SPECULA_CPU_GPU inline Xyz clamp_zero(Xyz xyz) {
    return Xyz(std::max<Float>(0, xyz.x), std::max<Float>(0, xyz.y), std::max<Float>(0, xyz.z));
  }

  /**
   * @brief Linear interpolation between two XYZ colors.
   *
   * @param t The interpolation factor, where 0 corresponds to `t0` and 1 corresponds to `t1`.
   * @param t0 The start XYZ color.
   * @param t1 The end XYZ color.
   * @return A new XYZ color that is the result of linear interpolation between `t0` and `t1`.
   */
  SPECULA_CPU_GPU inline Xyz lerp(Float t, Xyz t0, Xyz t1) { return (1 - t) * t0 + t * t1; }
} // namespace specula

template <> struct fmt::formatter<specula::Xyz> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Xyz &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ {} {} {} ]", v.x, v.y, v.z);
  }
};

#endif // INCLUDE_COLOR_XYZ_HPP_
