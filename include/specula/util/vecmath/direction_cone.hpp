#ifndef SPECULA_UTIL_VECMATH_DIRECTION_CONE_HPP
#define SPECULA_UTIL_VECMATH_DIRECTION_CONE_HPP

#include "specula/macros.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath/bounds3.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  class DirectionCode {
  public:
    DirectionCode() = default;
    SPECULA_CPU_GPU DirectionCode(Vector3f w, Float cos_theta)
        : w(normalize(w)), cos_theta(cos_theta) {}
    SPECULA_CPU_GPU explicit DirectionCode(Vector3f w) : DirectionCode(w, 1) {}

    SPECULA_CPU_GPU [[nodiscard]] bool is_empty() const { return cos_theta == INFINITY; }

    SPECULA_CPU_GPU static DirectionCode entire_sphere() { return {Vector3f(0, 0, 1), -1}; }

    SPECULA_CPU_GPU [[nodiscard]] Vector3f closest_vector_in_code(Vector3f wp) const {
      DASSERT(!is_empty());
      wp = normalize(wp);
      if (dot(wp, w) > cos_theta) {
        return wp;
      }

      Float sin_theta = -safe_sqrt(1 - cos_theta * cos_theta);
      Vector3f a = cross(wp, w);
      return cos_theta * w +
             (sin_theta / length(a)) *
                 Vector3f(w.x * (wp.y * w.y + wp.z * w.z) - wp.x * (sqr(w.y) + sqr(w.z)),
                          w.y * (wp.x * w.x + wp.z * w.z) - wp.y * (sqr(w.x) + sqr(w.z)),
                          w.z * (wp.x * w.x + wp.y * w.y) - wp.z * (sqr(w.x) + sqr(w.y)));
    }

    Vector3f w;
    Float cos_theta = INFINITY;
  };

  SPECULA_CPU_GPU inline bool inside(const DirectionCode &d, Vector3f w) {
    return !d.is_empty() && dot(d.w, normalize(w)) >= d.cos_theta;
  }

  SPECULA_CPU_GPU inline DirectionCode bound_subtended_directions(const Bounds3f &b, Point3f p) {
    Float radius = 0.f;
    Point3f p_center;
    b.bounding_sphere(&p_center, &radius);
    if (distance_squared(p, p_center) < sqr(radius)) {
      return DirectionCode::entire_sphere();
    }

    Vector3f w = normalize(p_center - p);
    Float sin2_theta_max = sqr(radius) / distance_squared(p_center, p);
    Float cos_theta_max = safe_sqrt(1 - sin2_theta_max);
    return {w, cos_theta_max};
  }

  SPECULA_CPU_GPU DirectionCode bunion(const DirectionCode &a, const DirectionCode &b);
} // namespace specula

template <> struct fmt::formatter<specula::DirectionCode> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::DirectionCode &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ DirectionCode w={} cosTheta={} ]", v.w, v.cos_theta);
  }
};

#endif // SPECULA_UTIL_VECMATH_DIRECTION_CONE_HPP
