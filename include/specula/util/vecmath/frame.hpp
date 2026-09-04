#ifndef SPECULA_UTIL_VECMATH_FRAME_HPP
#define SPECULA_UTIL_VECMATH_FRAME_HPP

#include "specula/macros.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  class Frame {
  public:
    SPECULA_CPU_GPU Frame() : x(1, 0, 0), y(0, 1, 0), z(0, 0, 1) {}
    SPECULA_CPU_GPU Frame(Vector3f x, Vector3f y, Vector3f z) : x(x), y(y), z(z) {
      DASSERT_LT(std::abs(length_squared(x) - 1), 1e-4);
      DASSERT_LT(std::abs(length_squared(y) - 1), 1e-4);
      DASSERT_LT(std::abs(length_squared(z) - 1), 1e-4);

      DASSERT_LT(std::abs(dot(x, y)), 1e-4);
      DASSERT_LT(std::abs(dot(y, z)), 1e-4);
      DASSERT_LT(std::abs(dot(z, x)), 1e-4);
    }

    SPECULA_CPU_GPU static Frame from_xz(Vector3f x, Vector3f z) { return {x, cross(z, x), z}; }
    SPECULA_CPU_GPU static Frame from_xy(Vector3f x, Vector3f y) { return {x, y, cross(x, y)}; }

    SPECULA_CPU_GPU static Frame from_x(Vector3f x) {
      Vector3f y, z;
      coordinate_system(x, &y, &z);
      return {x, y, z};
    }

    SPECULA_CPU_GPU static Frame from_y(Vector3f y) {
      Vector3f x, z;
      coordinate_system(y, &z, &x);
      return {x, y, z};
    }

    SPECULA_CPU_GPU static Frame from_z(Vector3f z) {
      Vector3f x, y;
      coordinate_system(z, &x, &y);
      return {x, y, z};
    }

    SPECULA_CPU_GPU static Frame from_x(Normal3f x) {
      Vector3f y, z;
      coordinate_system(x, &y, &z);
      return {Vector3f(x), y, z};
    }

    SPECULA_CPU_GPU static Frame from_y(Normal3f y) {
      Vector3f x, z;
      coordinate_system(y, &z, &x);
      return {x, Vector3f(y), z};
    }

    SPECULA_CPU_GPU static Frame from_z(Normal3f z) {
      Vector3f x, y;
      coordinate_system(z, &x, &y);
      return {x, y, Vector3f(z)};
    }

    SPECULA_CPU_GPU [[nodiscard]] Vector3f to_local(Vector3f v) const {
      return {dot(v, x), dot(v, y), dot(v, z)};
    }
    SPECULA_CPU_GPU [[nodiscard]] Normal3f to_local(Normal3f n) const {
      return {dot(n, x), dot(n, y), dot(n, z)};
    }

    SPECULA_CPU_GPU [[nodiscard]] Vector3f from_local(Vector3f v) const {
      return v.x * x + v.y * y + v.z * z;
    }
    SPECULA_CPU_GPU [[nodiscard]] Normal3f from_local(Normal3f n) const {
      return Normal3f(n.x * x + n.y * y + n.z * z);
    }

    Vector3f x, y, z;
  };
} // namespace specula

template <> struct fmt::formatter<specula::Frame> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext> auto format(const specula::Frame &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ Frame x={} y={} z={} ]", v.x, v.y, v.z);
  }
};

#endif // SPECULA_UTIL_VECMATH_FRAME_HPP
