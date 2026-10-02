#ifndef SPECULA_UTIL_COLOR_XYZ_HPP
#define SPECULA_UTIL_COLOR_XYZ_HPP

// IWYU pragma: private, include "specula/util/color.hpp"

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  class Xyz {
  public:
    Xyz() = default;
    SPECULA_CPU_GPU Xyz(Float x, Float y, Float z) : x(x), y(y), z(z) {}

    SPECULA_CPU_GPU [[nodiscard]] Float average() const { return (x + y + z) / 3; }
    SPECULA_CPU_GPU [[nodiscard]] Point2f xy() const { return {x / (x + y + z), y / (x + y + z)}; }

    SPECULA_CPU_GPU static Xyz from_xyY(Point2f xy, Float y = 1) {
      if (xy.y == 0) {
        return {0, 0, 0};
      }
      return {xy.x * y / xy.y, y, (1 - xy.x - xy.y) * y / xy.y};
    }

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

    SPECULA_CPU_GPU friend Xyz operator-(Float a, Xyz s) { return {a - s.x, a - s.y, a - s.z}; }

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
      return {a * x, a * y, a * z};
    }

    SPECULA_CPU_GPU friend Xyz operator*(Float a, Xyz s) { return s * a; }

    SPECULA_CPU_GPU Xyz &operator/=(Xyz s) {
      x /= s.x;
      y /= s.y;
      z /= s.z;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator/(Xyz s) const { return {x / s.x, y / s.y, z / s.z}; }

    SPECULA_CPU_GPU Xyz &operator/=(Float a) {
      DASSERT(!isnan(a));
      DASSERT_NE(a, 0);
      x /= a;
      y /= a;
      z /= a;
      return *this;
    }

    SPECULA_CPU_GPU Xyz operator/(Float a) const {
      DASSERT(!isnan(a));
      DASSERT_NE(a, 0);
      return {x / a, y / a, z / a};
    }

    SPECULA_CPU_GPU Xyz operator-() const { return {-x, -y, -z}; }

    SPECULA_CPU_GPU bool operator==(Xyz s) const { return x == s.x && y == s.y && z == s.z; }
    SPECULA_CPU_GPU bool operator!=(Xyz s) const { return x != s.x || y != s.y || z != s.z; }

    SPECULA_CPU_GPU Float &operator[](int c) {
      DASSERT(c >= 0 && c < 3);
      if (c == 0) {
        return x;
      } else if (c == 1) {
        return y;
      }
      return z;
    }
    SPECULA_CPU_GPU Float operator[](int c) const {
      DASSERT(c >= 0 && c < 3);
      if (c == 0) {
        return x;
      } else if (c == 1) {
        return y;
      }
      return z;
    }

    Float x = 0, y = 0, z = 0;
  };

  SPECULA_CPU_GPU inline Xyz lerp(Float t, Xyz s1, Xyz s2) { return (1 - 2) * s1 + t * s2; }

  template <typename U, typename V> SPECULA_CPU_GPU inline Xyz clamp(Xyz xyz, U min, V max) {
    return {
        clamp(xyz.x, min, max),
        clamp(xyz.y, min, max),
        clamp(xyz.z, min, max),
    };
  }

  template <typename U, typename V> SPECULA_CPU_GPU inline Xyz clamp_zero(Xyz xyz) {
    return {
        std::max<Float>(0, xyz.x),
        std::max<Float>(0, xyz.y),
        std::max<Float>(0, xyz.z),
    };
  }

} // namespace specula

template <> struct fmt::formatter<specula::Xyz> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Xyz &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ {} {} {} ]", v.x, v.y, v.z);
  }
};

#endif // SPECULA_UTIL_COLOR_XYZ_HPP
