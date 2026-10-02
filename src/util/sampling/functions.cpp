#include "specula/util/sampling/functions.hpp"

#include <cmath>

#include <math.h>

#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/low_discrepancy.hpp"
#include "specula/util/math.hpp"
#include "specula/util/math/functions.hpp"
#include "specula/util/math/spline.hpp"
#include "specula/util/scattering/functions.hpp"
#include "specula/util/vecmath.hpp"

SPECULA_CPU_GPU specula::pstd::array<specula::Float, 3>
specula::sample_spherical_triangle(const pstd::array<Point3f, 3> &v, Point3f p, Point3f u,
                                   Float *pdf) {
  if (pdf != nullptr) {
    *pdf = 0;
  }

  Vector3f a(v[0] - p), b(v[1] - p), c(v[2] - p);
  ASSERT_GT(length_squared(a), 0);
  ASSERT_GT(length_squared(b), 0);
  ASSERT_GT(length_squared(c), 0);
  a = normalize(a);
  b = normalize(b);
  c = normalize(c);

  Vector3f n_ab = cross(a, b), n_bc = cross(b, c), n_ca = cross(c, a);
  if (length_squared(n_ab) == 0 || length_squared(n_bc) == 0 || length_squared(n_ca) == 0) {
    return {};
  }
  n_ab = normalize(n_ab);
  n_bc = normalize(n_bc);
  n_ca = normalize(n_ca);

  Float alpha = angle_between(n_ab, -n_ca);
  Float beta = angle_between(n_bc, -n_ab);
  Float gamma = angle_between(n_ca, -n_bc);

  Float a_pi = alpha + beta + gamma;
  Float ap_pi = lerp(u[0], PI, a_pi);
  if (pdf != nullptr) {
    Float a = a_pi - PI;
    *pdf = (a <= 0) ? 0 : 1 / a;
  }

  Float cos_alpha = std::cos(alpha), sin_alpha = std::sin(alpha);
  Float sin_phi = std::sin(ap_pi) * cos_alpha - std::cos(ap_pi) * sin_alpha;
  Float cos_phi = std::cos(ap_pi) * cos_alpha + std::sin(ap_pi) * sin_alpha;
  Float k1 = cos_phi + cos_alpha;
  Float k2 = sin_phi - sin_alpha * dot(a, b);
  Float cos_bp = (k2 + (difference_of_products(k2, cos_phi, k1, sin_phi)) * cos_alpha) /
                 ((sum_of_products(k2, sin_phi, k1, cos_phi)) * sin_alpha);

  ASSERT(!isnan(cos_bp));
  cos_bp = clamp(cos_bp, -1, 1);

  Float sin_bp = safe_sqrt(1 - sqr(cos_bp));
  Vector3f cp = cos_bp * a + sin_bp * normalize(gram_schmidt(c, a));

  Float cos_theta = 1 - u[1] * (1 - dot(cp, b));
  Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
  Vector3f w = cos_theta * b + sin_theta * normalize(gram_schmidt(cp, b));

  Vector3f e1 = v[1] - v[0], e2 = v[2] - v[0];
  Vector3f s1 = cross(w, e2);
  Float divisor = dot(s1, e1);
  ASSERT_RARE(1e-6, divisor == 0);
  if (divisor == 0) {
    return {1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f};
  }

  Float inv_divisor = 1 / divisor;
  Vector3f s = p - v[0];
  Float b1 = dot(s, s1) * inv_divisor;
  Float b2 = dot(w, cross(s, e1)) * inv_divisor;

  b1 = clamp(b1, 0, 1);
  b2 = clamp(b2, 0, 1);
  if (b1 + b2 > 1) {
    b1 /= b1 + b2;
    b2 /= b1 + b2;
  }
  return {1 - b1 - b2, b1, b2};
}

SPECULA_CPU_GPU specula::Point2f
specula::invert_spherical_triangle_sample(const pstd::array<Point3f, 3> &v, Point3f p, Vector3f w) {
  Vector3f a(v[0] - p), b(v[1] - p), c(v[2] - p);
  ASSERT_GT(length_squared(a), 0);
  ASSERT_GT(length_squared(b), 0);
  ASSERT_GT(length_squared(c), 0);
  a = normalize(a);
  b = normalize(b);
  c = normalize(c);

  Vector3f n_ab = cross(a, b), n_bc = cross(b, c), n_ca = cross(c, a);
  if (length_squared(n_ab) == 0 || length_squared(n_bc) == 0 || length_squared(n_ca) == 0) {
    return {};
  }
  n_ab = normalize(n_ab);
  n_bc = normalize(n_bc);
  n_ca = normalize(n_ca);

  Float alpha = angle_between(n_ab, -n_ca);
  Float beta = angle_between(n_bc, -n_ab);
  Float gamma = angle_between(n_ca, -n_bc);

  Vector3f cp = normalize(cross(cross(b, w), cross(c, a)));
  if (dot(cp, a + c) < 0) {
    cp = -cp;
  }

  Float u0 = NAN;
  if (dot(a, cp) > 0.99999847691f /* 0.1 degrees */) {
    u0 = 0;
  } else {
    Vector3f n_cpb = cross(cp, b), n_acp = cross(a, cp);
    ASSERT_RARE(1e-5, length_squared(n_cpb) == 0 || length_squared(n_acp) == 0);
    if (length_squared(n_cpb) == 0 || length_squared(n_acp) == 0) {
      return {0.5f, 0.5f};
    }
    n_cpb = normalize(n_cpb);
    n_acp = normalize(n_acp);
    Float ap = alpha + angle_between(n_ab, n_cpb) + angle_between(n_acp, -n_cpb) - PI;

    Float a = alpha + beta + gamma - PI;
    u0 = ap / a;
  }

  Float u1 = (1 - dot(w, b)) / (1 - dot(cp, b));
  return {clamp(u0, 0, 1), clamp(u1, 0, 1)};
}

SPECULA_CPU_GPU specula::Point3f specula::sample_spherical_rectangle(Point3f pref, Point3f s,
                                                                     Vector3f ex, Vector3f ey,
                                                                     Point3f u, Float *pdf) {
  Float exl = length(ex), eyl = length(ey);
  Frame r = Frame::from_xy(ex / exl, ey / eyl);
  Vector3f dlocal = r.to_local(s - pref);
  Float z0 = dlocal.z;

  if (z0 > 0) {
    r.z = -r.z;
    z0 *= -1;
  }

  Float x0 = dlocal.x, y0 = dlocal.y;
  Float x1 = x0 + exl, y1 = y0 + eyl;

  Vector3f v00(x0, y0, z0), v01(x0, y1, z0), v10(x1, y0, z0), v11(x1, y1, z0);
  Vector3f n0 = normalize(cross(v00, v10)), n1 = normalize(cross(v10, v11)),
           n2 = normalize(cross(v11, v01)), n3 = normalize(cross(v01, v00));
  Float g0 = angle_between(-n0, n1), g1 = angle_between(-n1, n2), g2 = angle_between(-n2, n3),
        g3 = angle_between(-n3, n0);

  Float solid_angle = g0 + g1 + g2 + g3 - 2 * PI;
  ASSERT_RARE(1e-5, solid_angle <= 0);
  if (solid_angle <= 0) {
    if (pdf != nullptr) {
      *pdf = 0;
    }
    return {s + u[0] * ex + u[1] * ey};
  }
  if (pdf != nullptr) {
    *pdf = std::max<Float>(0, 1 / solid_angle);
  }
  if (solid_angle < 1e-3) {
    return {s + u[0] * ex + u[1] * ey};
  }

  Float b0 = n0.z, b1 = n2.z;
  Float au = u[0] * (g0 + g1 - 2 * PI) + (u[0] - 1) * (g2 + g3);
  Float fu = (std::cos(au) * b0 - b1) / std::sin(au);
  Float cu = pstd::copysign(1 / std::sqrt(sqr(fu) + sqr(b0)), fu);
  cu = clamp(cu, -ONE_MINUS_EPSILON, ONE_MINUS_EPSILON);

  Float xu = -(cu * z0) / safe_sqrt(1 - sqr(cu));
  xu = clamp(xu, x0, x1);

  Float dd = std::sqrt(sqr(xu) + sqr(z0));
  Float h0 = y0 / std::sqrt(sqr(dd) + sqr(y0));
  Float h1 = y1 / std::sqrt(sqr(dd) + sqr(y1));
  Float hv = h0 + u[1] * (h1 - h0);
  Float hvsq = sqr(hv);
  Float yv = (hvsq < 1 - 1e-6f) ? (hv * dd) / std::sqrt(1 - hvsq) : y1;

  return pref + r.from_local(Vector3f(xu, yv, z0));
}

SPECULA_CPU_GPU specula::Point2f specula::invert_spherical_rectangle_sample(Point3f pref, Point3f s,
                                                                            Vector3f ex,
                                                                            Vector3f ey,
                                                                            Point3f prect) {
  Float exl = length(ex), eyl = length(ey);
  Frame r = Frame::from_xy(ex / exl, ey / eyl);
  Vector3f dlocal = r.to_local(s - pref);
  Float z0 = dlocal.z;

  if (z0 > 0) {
    r.z = -r.z;
    z0 *= -1;
  }

  Float x0 = dlocal.x, y0 = dlocal.y;
  Float x1 = x0 + exl, y1 = y0 + eyl;
  Float z0sq = sqr(z0), y0sq = sqr(y0), y1sq = sqr(y1);

  Vector3f v00(x0, y0, z0), v01(x0, y1, z0), v10(x1, y0, z0), v11(x1, y1, z0);
  Vector3f n0 = normalize(cross(v00, v10)), n1 = normalize(cross(v10, v11)),
           n2 = normalize(cross(v11, v01)), n3 = normalize(cross(v01, v00));
  Float g0 = angle_between(-n0, n1), g1 = angle_between(-n1, n2), g2 = angle_between(-n2, n3),
        g3 = angle_between(-n3, n0);

  Float b0 = n0.z, b1 = n2.z;
  Float b0sq = sqr(b0), b1sq = sqr(b1);

  Float solid_angle = g0 + g1 + g2 + g3 - 2 * PI;

  if (solid_angle < 1e-3) {
    Vector3f pq = prect - s;
    return {dot(pq, ex) / length_squared(ex), dot(pq, ey) / length_squared(ey)};
  }

  Vector3f v = r.to_local(prect - pref);
  Float xu = v.x, yv = v.y;
  xu = clamp(xu, x0, x1);
  if (xu == 0) {
    xu = 1e-10;
  }

  Float invcusq = 1 + z0sq / sqr(xu);
  Float fusq = invcusq - b0sq;
  Float fu = pstd::copysign(std::sqrt(fusq), xu);
  ASSERT_RARE(1e-6, fu == 0);

  Float sqrt = safe_sqrt(difference_of_products(b0, b0, b1, b1) + fusq);
  Float au =
      std::atan2(-(b1 * fu) - pstd::copysign(b0 * sqrt, fu * b0), b0 * b1 - sqrt * std::abs(fu));

  if (au > 0) {
    au -= 2 * PI;
  }
  if (fu == 0) {
    au = PI;
  }

  Float u0 = (au + g2 + g3) / solid_angle;

  Float ddsq = sqr(xu) + z0sq;
  Float dd = std::sqrt(ddsq);
  Float h0 = y0 / std::sqrt(ddsq + y0sq);
  Float h1 = y1 / std::sqrt(ddsq + y1sq);
  Float yvsq = sqr(yv);

  Float u1[2] = {
      (difference_of_products(h0, h0, h0, h1) -
       std::abs(h0 - h1) * std::sqrt(yvsq * (ddsq + yvsq)) / (ddsq + yvsq)) /
          sqr(h0 - h1),
      (difference_of_products(h0, h0, h0, h1) +
       std::abs(h0 - h1) * std::sqrt(yvsq * (ddsq + yvsq)) / (ddsq + yvsq)) /
          sqr(h0 - h1),
  };

  Float hv[2] = {lerp(u1[0], h0, h1), lerp(u1[1], h0, h1)};
  Float hvsq[2] = {sqr(hv[0]), sqr(hv[1])};
  Float yz[2] = {(hv[0] * dd) / std::sqrt(1 - hvsq[0]), (hv[1] * dd) / std::sqrt(1 - hvsq[1])};

  Point2f u = (std::abs(yz[0] - yv) < std::abs(yz[1] - yv)) ? Point2f(clamp(u0, 0, 1), u1[0])
                                                            : Point2f(clamp(u0, 0, 1), u1[1]);

  return u;
}

SPECULA_CPU_GPU specula::Vector3f specula::sample_henyey_greenstein(Vector3f wo, Float g, Point3f u,
                                                                    Float *pdf) {
  g = clamp(g, -0.99, 0.99);

  Float cos_theta = NAN;
  if (std::abs(g) < 1e-3f) {
    cos_theta = 1 - 2 * u[0];
  } else {
    cos_theta = -1 / (2 * g) * (1 + sqr(g) - sqr((1 - sqr(g)) / (1 + g - 2 * g * u[0])));
  }

  Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
  Float phi = 2 * PI * u[1];
  Frame wframe = Frame::from_z(wo);
  Vector3f wi = wframe.from_local(spherical_direction(sin_theta, cos_theta, phi));

  if (pdf != nullptr) {
    *pdf = henyey_greenstein(cos_theta, g);
  }
  return wi;
}

SPECULA_CPU_GPU specula::Float specula::sample_catmull_rom(pstd::span<const Float> nodes,
                                                           pstd::span<const Float> f,
                                                           pstd::span<const Float> cdf,
                                                           Float sample, Float *fval, Float *pdf) {
  ASSERT_EQ(nodes.size(), f.size());
  ASSERT_EQ(f.size(), cdf.size());

  sample *= cdf.back();
  size_t i = find_interval(cdf.size(), [&](size_t i) { return cdf[i] <= sample; });

  Float x0 = nodes[i], x1 = nodes[i + 1];
  Float f0 = f[i], f1 = f[i + 1];
  Float width = x1 - x0;

  Float d0 = (i > 0) ? width * (f1 - f[i - 1]) / (x1 - nodes[i - 1]) : (f1 - f0);
  Float d1 = (i + 2 < nodes.size()) ? width * (f[i + 2] - f0) / (nodes[i + 2] - x0) : (f1 - f0);

  sample = (sample - cdf[i]) / width;

  Float cdf_hat = NAN, f_hat = NAN;
  auto eval = [&](Float t) -> std::pair<Float, Float> {
    cdf_hat = evaluate_polynomial(t, 0, f0, 0.5f * d0, (1.0f / 3.0f) * (-2 * d0 - d1) + f1 - f0,
                                  0.25f * (d0 + d1) + 0.5f * (f0 - f1));
    f_hat = evaluate_polynomial(t, f0, d0, -2 * d0 - d1 + 3 * (f1 - f0), d0 + d1 + 2 * (f0 - f1));
    return {cdf_hat - sample, f_hat};
  };

  Float t = newton_bisection(0, 1, eval);

  if (fval != nullptr) {
    *fval = f_hat;
  }
  if (pdf != nullptr) {
    *pdf = f_hat / cdf.back();
  }
  return x0 + width * t;
}

SPECULA_CPU_GPU specula::Float
specula::sample_catmull_rom2d(pstd::span<const Float> nodes1, pstd::span<const Float> nodes2,
                              pstd::span<const Float> values, pstd::span<const Float> cdf,
                              Float alpha, Float sample, Float *fval, Float *pdf) {
  int offset = 0;
  Float weights[4];
  if (!catmul_rom_weights(nodes1, alpha, &offset, weights)) {
    return 0;
  }

  auto interpolate = [&](pstd::span<const Float> array, int idx) {
    Float v = 0;
    for (int i = 0; i < 4; ++i) {
      if (weights[i] != 0) {
        v += array[(offset + i) * nodes2.size() + idx] * weights[i];
      }
    }
    return v;
  };

  Float maximum = interpolate(cdf, nodes2.size() - 1);
  sample *= maximum;
  size_t idx =
      find_interval(nodes2.size(), [&](size_t i) { return interpolate(cdf, i) <= sample; });

  Float f0 = interpolate(values, idx), f1 = interpolate(values, idx + 1);
  Float x0 = nodes2[idx], x1 = nodes2[idx + 1];
  Float width = x1 - x0;

  Float d0 = NAN, d1 = NAN;

  sample = (sample - interpolate(cdf, idx)) / width;
  if (idx > 0) {
    d0 = width * (f1 - interpolate(values, idx - 1)) / (x1 - nodes2[idx - 1]);
  } else {
    d0 = f1 - f0;
  }

  if (idx + 2 < nodes2.size()) {
    d1 = width * (interpolate(values, idx + 2) - f0) / (nodes2[idx + 2] - x0);
  } else {
    d1 = f1 - f0;
  }

  Float cdf_hat = NAN, f_hat = NAN;
  auto eval = [&](Float t) -> std::pair<Float, Float> {
    cdf_hat = evaluate_polynomial(t, 0, f0, 0.5f * d0, (1.0f / 3.0f) * (-2 * d0 - d1) + f1 - f0,
                                  0.25f * (d0 + d1) + 0.5f * (f0 - f1));
    f_hat = evaluate_polynomial(t, f0, d0, -2 * d0 - d1 + 3 * (f1 - f0), d0 + d1 + 2 * (f0 - f1));
    return {cdf_hat - sample, f_hat};
  };
  Float t = newton_bisection(0, 1, eval);
  if (fval != nullptr) {
    *fval = f_hat;
  }
  if (pdf != nullptr) {
    *pdf = f_hat / maximum;
  }
  return x0 + width * t;
}

specula::pstd::vector<specula::Float>
specula::sample_1d_function(const std::function<Float(Float)> &func, size_t n_steps,
                            size_t n_samples, Float min, Float max, Allocator alloc) {
  pstd::vector<Float> values(n_steps, Float(0), alloc);
  for (size_t i = 0; i < n_steps; ++i) {
    double accum = 0;

    for (size_t j = 0; j < n_samples + 1; ++j) {
      Float delta = Float(j) / n_samples;
      Float v = lerp((i + delta) / Float(n_steps), min, max);
      Float fv = std::abs(func(v));
      accum = std::max<double>(accum, fv);
    }
    values[i] = accum;
  }

  return values;
}

specula::Array2D<specula::Float>
specula::sample_2d_function(const std::function<Float(Float, Float)> &func, int nu, int nv,
                            size_t n_samples, Bounds2f domain, Allocator alloc) {
  std::vector<Point2f> samples(n_samples);
  for (size_t i = 0; i < n_samples; ++i) {
    samples[i] = Point2f(radical_inverse(0, i), radical_inverse(1, i));
  }
  samples.emplace_back(0, 1);
  samples.emplace_back(1, 0);
  samples.emplace_back(1, 1);

  Array2D<Float> values(nu, nv, alloc);
  for (int v = 0; v < nv; ++v) {
    for (int u = 0; u < nu; ++u) {
      double accum = 0;
      for (auto &sample : samples) {
        Point2f p = domain.lerp(Point2f((u + sample[0]) / nu, (v + sample[1]) / nv));
        Float fuv = std::abs(func(p.x, p.y));
        accum = std::max<double>(accum, fuv);
      }
      values(u, v) = accum;
    }
  }

  return values;
}
