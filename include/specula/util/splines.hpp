#ifndef SPECULA_UTIL_SPLINES_HPP
#define SPECULA_UTIL_SPLINES_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/math.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  template <typename P>
  SPECULA_CPU_GPU inline P blossom_cubic_bezier(pstd::span<const P> p, Float u0, Float u1,
                                                Float u2) {
    P a[3] = {lerp(u0, p[0], p[1]), lerp(u0, p[1], p[2]), lerp(u0, p[2], p[3])};
    P b[2] = {lerp(u1, a[0], a[1]), lerp(u1, a[1], a[2])};
    return lerp(u2, b[0], b[1]);
  }

  template <typename P>
  SPECULA_CPU_GPU inline P evaluate_cubic_bezier(pstd::span<const P> cp, Float u) {
    return blossom_cubic_bezier(cp, u, u, u);
  }

  SPECULA_CPU_GPU inline Point3f evaluate_cubic_bezier(pstd::span<const Point3f> cp, Float u,
                                                       Vector3f *deriv) {
    Point3f a[3] = {lerp(u, cp[0], cp[1]), lerp(u, cp[1], cp[2]), lerp(u, cp[2], cp[3])};
    Point3f b[2] = {lerp(u, a[0], a[1]), lerp(u, a[1], a[2])};
    if (deriv != nullptr) {
      if (length_squared(b[1] - b[0]) > 0) {
        *deriv = 3 * (b[1] - b[0]);
      } else {
        *deriv = cp[3] - cp[0];
      }
    }
    return lerp(u, b[0], b[1]);
  }

  SPECULA_CPU_GPU inline pstd::array<Point3f, 7>
  subdivide_cubic_bezier(pstd::span<const Point3f> cp) {
    return {cp[0],
            (cp[0] + cp[1]) / 2,
            (cp[0] + 2 * cp[1] + cp[2]) / 4,
            (cp[0] + 3 * cp[1] + 3 * cp[2] + cp[3]) / 8,
            (cp[1] + 2 * cp[2] + cp[3]) / 4,
            (cp[2] + cp[3]) / 2,
            cp[3]};
  }

  SPECULA_CPU_GPU inline pstd::array<Point3f, 4>
  cubic_bezier_control_points(pstd::span<const Point3f> cp, Float u_min, Float u_max) {
    return {
        blossom_cubic_bezier(cp, u_min, u_min, u_min),
        blossom_cubic_bezier(cp, u_min, u_min, u_max),
        blossom_cubic_bezier(cp, u_min, u_max, u_max),
        blossom_cubic_bezier(cp, u_max, u_max, u_max),
    };
  }

  SPECULA_CPU_GPU inline Bounds3f bound_cubic_bezier(pstd::span<const Point3f> cp) {
    return bunion(Bounds3f(cp[0], cp[1]), Bounds3f(cp[2], cp[3]));
  }

  SPECULA_CPU_GPU inline Bounds3f bound_cubic_bezier(pstd::span<const Point3f> cp, Float u_min,
                                                     Float u_max) {
    if (u_min == 0 && u_max == 0) {
      return bound_cubic_bezier(cp);
    }
    auto cp_seg = cubic_bezier_control_points(cp, u_min, u_max);
    return bound_cubic_bezier(pstd::span<const Point3f>(cp_seg));
  }

  SPECULA_CPU_GPU inline pstd::array<Point3f, 3>
  elevate_quadratic_bezier_to_cubic(pstd::span<const Point3f> cp) {
    return {cp[0], lerp(2.0f / 3.0f, cp[0], cp[1]), lerp(1.0f / 3.0f, cp[1], cp[2]), cp[2]};
  }

  SPECULA_CPU_GPU inline pstd::array<Point3f, 3>
  quadratic_bspline_to_bezier(pstd::span<const Point3f> cp) {
    Point3f p11 = lerp(0.5f, cp[0], cp[1]);
    Point3f p22 = lerp(0.5f, cp[1], cp[2]);
    return {p11, cp[1], p22};
  }

  SPECULA_CPU_GPU inline pstd::array<Point3f, 4>
  cubic_bspline_to_bezier(pstd::span<const Point3f> cp) {
    Point3f p012 = cp[0], p123 = cp[1], p234 = cp[2], p345 = cp[3];

    Point3f p122 = lerp(2.0f / 3.0f, p012, p123);
    Point3f p223 = lerp(1.0f / 3.0f, p123, p234);
    Point3f p233 = lerp(2.0f / 3.0f, p123, p234);
    Point3f p334 = lerp(1.0f / 3.0f, p234, p345);

    Point3f p222 = lerp(0.5f, p122, p223);
    Point3f p333 = lerp(0.5f, p233, p334);

    return {p222, p223, p233, p333};
  }
} // namespace specula

#endif // SPECULA_UTIL_SPLINES_HPP
