#include "specula/util/vecmath/equal_area.hpp"

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/math.hpp"

SPECULA_CPU_GPU specula::Vector3f specula::equal_area_square_to_sphere(Point2f p) {
  ASSERT(p.x >= 0 && p.x <= 1 && p.y >= 0 && p.y <= 1);
  Float u = 2 * p.x - 1, v = 2 * p.y - 1;
  Float up = std::abs(u), vp = std::abs(v);

  Float signed_distance = 1 - (up + vp);
  Float d = std::abs(signed_distance);
  Float r = 1 - d;

  Float phi = (r == 0 ? 1 : (vp - up) / r + 1) * PI / 4;

  Float z = pstd::copysign(1 - sqr(r), signed_distance);

  Float cos_phi = std::copysign(std::cos(phi), u);
  Float sin_phi = std::copysign(std::sin(phi), v);

  return {cos_phi * r * safe_sqrt(2 - sqr(r)), sin_phi * r * safe_sqrt(2 - sqr(r)), z};
}

SPECULA_CPU_GPU specula::Point2f specula::equal_area_sphere_to_square(Vector3f d) {
  DASSERT(length_squared(d) > 0.999 && length_squared(d) < 1.001);
  Float x = std::abs(d.x), y = std::abs(d.y), z = std::abs(d.z);

  Float r = safe_sqrt(1 - z);

  Float a = std::max(x, y), b = std::min(x, y);
  b = a == 0 ? 0 : b / a;

  const Float t1 = 0.406758566246788489601959989e-5;
  const Float t2 = 0.636226545274016134946890922156;
  const Float t3 = 0.61572017898280213493197203466e-2;
  const Float t4 = -0.247333733281268944196501420480;
  const Float t5 = 0.881770664775316294736387951347e-1;
  const Float t6 = 0.419038818029165735901852432784e-1;
  const Float t7 = -0.251390972343483509333252996350e-1;

  Float phi = evaluate_polynomial(b, t1, t2, t3, t4, t5, t6, t7);
  if (x < y) {
    phi = 1 - phi;
  }

  Float v = phi * r;
  Float u = r - v;

  if (d.z < 0) {
    pstd::swap(u, v);
    u = 1 - u;
    v = 1 - v;
  }

  u = pstd::copysign(u, d.x);
  v = pstd::copysign(v, d.y);

  return {0.5f * (u + 1), 0.5f * (v + 1)};
}

SPECULA_CPU_GPU specula::Point2f specula::wrap_equal_area_square(Point2f uv) {
  if (uv[0] < 0) {
    uv[0] = -uv[0];
    uv[1] = 1 - uv[1];
  } else if (uv[0] > 1) {
    uv[0] = 2 - uv[0];
    uv[1] = 1 - uv[1];
  }

  if (uv[1] < 0) {
    uv[0] = 1 - uv[0];
    uv[1] = -uv[1];
  } else if (uv[1] > 1) {
    uv[0] = 1 - uv[0];
    uv[1] = 2 - uv[1];
  }

  return uv;
}
