#include "specula/util/vecmath/direction_cone.hpp"

#include "specula/macros.hpp"
#include "specula/util/vecmath/tuple3.hpp"

SPECULA_CPU_GPU specula::DirectionCode specula::bunion(const DirectionCode &a,
                                                       const DirectionCode &b) {
  if (a.is_empty()) {
    return b;
  }
  if (b.is_empty()) {
    return a;
  }

  Float theta_a = safe_acos(a.cos_theta), theta_b = safe_acos(b.cos_theta),
        theta_d = angle_between(a.w, b.w);

  if (std::min(theta_d + theta_b, PI) <= theta_a) {
    return a;
  }
  if (std::min(theta_d + theta_a, PI) <= theta_b) {
    return b;
  }

  Float theta_o = (theta_a + theta_b + theta_d) / 2;
  if (theta_o >= PI) {
    return DirectionCode::entire_sphere();
  }

  Float theta_r = theta_o - theta_a;
  Vector3f wr = cross(a.w, b.w);
  if (length_squared(wr) == 0) {
    return DirectionCode::entire_sphere();
  }
  // TODO: Implement the transform.hpp header for 'rotate'
  // Vector3f w = rotate(degrees(theta_r), wr)(a.w);
  Vector3f w = a.w;
  return {w, std::cos(theta_o)};
}
