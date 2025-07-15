#ifndef INCLUDE_COLOR_XYZ_HPP_
#define INCLUDE_COLOR_XYZ_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {
  class Xyz {
  public:
    Xyz() = default;
    SPECULA_CPU_GPU Xyz(Float x, Float y, Float z) : x(x), y(y), z(z) {}

    SPECULA_CPU_GPU static Xyz from_xyY(Point2f xy, Float y = 1) {
      if (xy.y == 0)
        return Xyz(0.0, 0.0, 0.0);
      return Xyz(xy.x * y / xy.y, y, (1 - xy.x - xy.y) * y / xy.y);
    }

    SPECULA_CPU_GPU Float average() const { return (x + y + z) / 3.0f; }
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

  template <typename U, typename V> SPECULA_CPU_GPU inline Xyz clamp(Xyz xyz, U min, V max) {
    return Xyz(clamp(xyz.x, min, max), clamp(xyz.y, min, max), clamp(xyz.z, min, max));
  }

  template <typename U, typename V> SPECULA_CPU_GPU inline Xyz clamp_zero(Xyz xyz) {
    return Xyz(std::max<Float>(0, xyz.x), std::max<Float>(0, xyz.y), std::max<Float>(0, xyz.z));
  }

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
