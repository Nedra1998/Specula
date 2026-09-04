#ifndef SPECULA_UTIL_VECMATH_SPHERICAL_GEOMETRY_HPP
#define SPECULA_UTIL_VECMATH_SPHERICAL_GEOMETRY_HPP

#include "specula/macros.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  SPECULA_CPU_GPU inline Float spherical_triangle_area(Vector3f a, Vector3f b, Vector3f c) {
    return std::abs(2 * std::atan2(dot(a, cross(b, c)), 1 + dot(a, b) + dot(a, c) + dot(b, c)));
  }

  SPECULA_CPU_GPU inline Float spherical_quad_area(Vector3f a, Vector3f b, Vector3f c, Vector3f d) {
    Vector3f axb = cross(a, b), bxc = cross(b, c), cxd = cross(c, d), dxa = cross(d, a);
    if (length_squared(axb) == 0 || length_squared(bxc) == 0 || length_squared(cxd) == 0 ||
        length_squared(dxa) == 0) {
      return 0;
    }
    axb = normalize(axb);
    bxc = normalize(bxc);
    cxd = normalize(cxd);
    dxa = normalize(dxa);

    Float alpha = angle_between(dxa, -axb);
    Float beta = angle_between(axb, -bxc);
    Float gamma = angle_between(bxc, -cxd);
    Float delta = angle_between(cxd, -dxa);

    return std::abs(alpha + beta + gamma + delta - 2 * PI);
  }

  SPECULA_CPU_GPU inline Vector3f spherical_direction(Float sin_theta, Float cos_theta, Float phi) {
    DASSERT(sin_theta >= -1.0001 && sin_theta <= 1.0001);
    DASSERT(cos_theta >= -1.0001 && cos_theta <= 1.0001);
    return {clamp(sin_theta, -1, 1) * std::cos(phi), clamp(sin_theta, -1, 1) * std::sin(phi),
            clamp(cos_theta, -1, 1)};
  }

  SPECULA_CPU_GPU inline Float spherical_theta(Vector3f v) { return safe_acos(v.z); }
  SPECULA_CPU_GPU inline Float spherical_phi(Vector3f v) {
    Float p = std::atan2(v.y, v.z);
    return (p < 0) ? (p + 2 * PI) : p;
  }

  SPECULA_CPU_GPU inline Float cos_theta(Vector3f w) { return w.z; }
  SPECULA_CPU_GPU inline Float cos2_theta(Vector3f w) { return sqr(w.z); }
  SPECULA_CPU_GPU inline Float abs_cos_theta(Vector3f w) { return std::abs(w.z); }
  SPECULA_CPU_GPU inline Float sin2_theta(Vector3f w) {
    return std::max<Float>(0, 1 - cos2_theta(w));
  }
  SPECULA_CPU_GPU inline Float sin_theta(Vector3f w) { return std::sqrt(sin2_theta(w)); }
  SPECULA_CPU_GPU inline Float tan_theta(Vector3f w) { return sin_theta(w) / cos_theta(w); }
  SPECULA_CPU_GPU inline Float tan2_theta(Vector3f w) { return sin2_theta(w) / cos2_theta(w); }

  SPECULA_CPU_GPU inline Float cos_phi(Vector3f w) {
    Float sin_theta = specula::sin_theta(w);
    return (sin_theta == 0) ? 1 : clamp(w.x / sin_theta, -1, 1);
  }
  SPECULA_CPU_GPU inline Float sin_phi(Vector3f w) {
    Float sin_theta = specula::sin_theta(w);
    return (sin_theta == 0) ? 0 : clamp(w.y / sin_theta, -1, 1);
  }

  SPECULA_CPU_GPU inline Float cosd_phi(Vector3f wa, Vector3f wb) {
    Float waxy = sqr(wa.x) + sqr(wa.y), wbxy = sqr(wb.x) + sqr(wb.y);
    if (waxy == 0 || wbxy == 0) {
      return 1;
    }
    return clamp((wa.x * wb.x + wa.y * wb.y) / std::sqrt(waxy * wbxy), -1, 1);
  }

  SPECULA_CPU_GPU inline bool same_hemisphere(Vector3f w, Vector3f wp) { return w.z * wp.z > 0; }

  SPECULA_CPU_GPU inline bool same_hemisphere(Vector3f w, Normal3f wp) { return w.z * wp.z > 0; }
} // namespace specula

#endif // SPECULA_UTIL_VECMATH_SPHERICAL_GEOMETRY_HPP
