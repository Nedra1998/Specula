#ifndef SPECULA_UTIL_SAMPLING_FUNCTIONS_HPP
#define SPECULA_UTIL_SAMPLING_FUNCTIONS_HPP

#include <cmath>

#include <fmt/base.h>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/containers.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math.hpp"
#include "specula/util/noise.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  SPECULA_CPU_GPU inline Float balance_heuristic(int nf, Float fpdf, int ng, Float gpdf) {
    return (nf * fpdf) / (nf * fpdf + ng * gpdf);
  }

  SPECULA_CPU_GPU inline Float power_heuristic(int nf, Float fpdf, int ng, Float gpdf) {
    Float f = nf * fpdf, g = ng * gpdf;
    if (isinf(sqr(f))) {
      return 1;
    }
    return sqr(f) / (sqr(f) + sqr(g));
  }

  SPECULA_CPU_GPU inline int sample_discrete(pstd::span<const Float> weights, Float u, Float *pmf,
                                             Float *uremapped) {
    if (weights.empty()) {
      if (pmf != nullptr) {
        *pmf = 0;
      }
      return -1;
    }

    Float sum_weights = 0;
    for (Float w : weights) {
      sum_weights += w;
    }
    Float up = u * sum_weights;
    if (up == sum_weights) {
      up = next_float_down(up);
    }

    int offset = 0;
    Float sum = 0;
    while (sum + weights[offset] <= up) {
      sum += weights[offset++];
      DASSERT_LT(offset, weights.size());
    }

    if (pmf != nullptr) {
      *pmf = weights[offset] / sum_weights;
    }

    if (uremapped != nullptr) {
      *uremapped = std::min<Float>((up - sum) / weights[offset], ONE_MINUS_EPSILON);
    }

    return offset;
  }

  SPECULA_CPU_GPU inline Float linear_pdf(Float x, Float a, Float b) {
    DASSERT(a >= 0 && b >= 0);
    if (x < 0 || x > 1) {
      return 0;
    }
    return 2 * lerp(x, a, b) / (a + b);
  }

  SPECULA_CPU_GPU inline Float sample_linear(Float u, Float a, Float b) {
    DASSERT(a >= 0 && b >= 0);
    if (u == 0 && a == 0) {
      return 0;
    }
    Float x = u * (a + b) / (a + std::sqrt(lerp(u, sqr(a), sqr(b))));
    return std::min<Float>(x, ONE_MINUS_EPSILON);
  }

  SPECULA_CPU_GPU inline Float invert_linear_sample(Float x, Float a, Float b) {
    return x * (a * (2 - x) + b * x) / (a + b);
  }

  SPECULA_CPU_GPU inline Float bilinear_pdf(Point2f p, pstd::span<const Float> w) {
    DASSERT_EQ(4, w.size());
    if (p.x < 0 || p.x > 1 || p.y < 0 || p.y > 1) {
      return 0;
    }
    if (w[0] + w[1] + w[2] + w[3] == 0) {
      return 1;
    }

    return 4 *
           ((1 - p[0]) * (1 - p[1]) * w[0] + p[0] * (1 - p[1]) * w[1] + (1 - p[0]) * p[1] * w[2] +
            p[0] * p[1] * w[3]) /
           (w[0] + w[1] + w[2] + w[3]);
  }

  SPECULA_CPU_GPU inline Point2f sample_bilinear(Point2f u, pstd::span<const Float> w) {
    DASSERT_EQ(4, w.size());
    Point2f p;
    p.y = sample_linear(u[1], w[0] + w[1], w[2] + w[3]);
    p.x = sample_linear(u[0], lerp(p.y, w[0], w[2]), lerp(p.y, w[1], w[3]));
    return p;
  }

  SPECULA_CPU_GPU inline Point2f invert_bilinear_sample(Point2f p, pstd::span<const Float> w) {
    return {invert_linear_sample(p.x, lerp(p.y, w[0], w[2]), lerp(p.y, w[1], w[3])),
            invert_linear_sample(p.y, w[0] + w[1], w[2] + w[3])};
  }

  SPECULA_CPU_GPU inline Float visible_wavelengths_pdf(Float lambda) {
    if (lambda < 360 || lambda > 830) {
      return 0;
    }
    return 0.0039398042f / sqr(std::cosh(0.0072f * (lambda - 538)));
  }

  SPECULA_CPU_GPU inline Float sample_visible_wavelengths(Float u) {
    return 538 - 138.888889f * std::atanh(0.85691062f - 1.82750197f * u);
  }

  SPECULA_CPU_GPU inline pstd::array<Float, 3> sample_uniform_triangle3(Point2f u) {
    Float b0 = NAN, b1 = NAN;
    if (u[0] < u[1]) {
      b0 = u[0] / 2;
      b1 = u[1] - b0;
    } else {
      b1 = u[1] / 2;
      b0 = u[0] - b1;
    }
    return {b0, b1, 1 - b0 - b1};
  }

  SPECULA_CPU_GPU inline Point2f invert_uniform_triangle_sample(const pstd::array<Float, 3> &b) {
    if (b[0] > b[1]) {
      return {b[0] + b[1], 2 * b[1]};
    } else {
      return {2 * b[0], b[1] + b[2]};
    }
  }

  SPECULA_CPU_GPU inline Float sample_tent(Float u, Float r) {
    if (sample_discrete({0.5f, 0.5f}, u, nullptr, &u) == 0) {
      return -r + r * sample_linear(u, 0, 1);
    } else {
      return r * sample_linear(u, 1, 0);
    }
  }

  SPECULA_CPU_GPU inline Float tent_pdf(Float x, Float r) {
    if (std::abs(x) >= r) {
      return 0;
    }
    return 1 / r - std::abs(x) / sqr(r);
  }

  SPECULA_CPU_GPU inline Float invert_tent_sample(Float x, Float r) {
    if (x <= 0) {
      return (1 - invert_linear_sample(-x / r, 1, 0)) / 2;
    } else {
      return 0.5f + invert_linear_sample(x / r, 1, 0) / 2;
    }
  }

  SPECULA_CPU_GPU inline Float exponential_pdf(Float x, Float a) {
    DASSERT_GT(a, 0);
    return a * std::exp(-a * x);
  }

  SPECULA_CPU_GPU inline Float sample_exponential(Float u, Float a) {
    DASSERT_GT(a, 0);
    return -std::log(1 - u) / a;
  }

  SPECULA_CPU_GPU inline Float invert_exponential_sample(Float x, Float a) {
    DASSERT_GT(a, 0);
    return 1 - std::exp(-a * x);
  }

  SPECULA_CPU_GPU inline Float normal_pdf(Float x, Float mu = 0, Float sigma = 1) {
    return gaussian(x, mu, sigma);
  }

  SPECULA_CPU_GPU inline Float sample_normal(Float u, Float mu = 0, Float sigma = 1) {
    return mu + SQRT2 * sigma * erf_inv(2 * u - 1);
  }

  SPECULA_CPU_GPU inline Float invert_normal_sample(Float x, Float mu = 0, Float sigma = 1) {
    return 0.5f * (1 + std::erf((x - mu) / (sigma * SQRT2)));
  }

  SPECULA_CPU_GPU inline Point2f sample_two_normal(Point2f u, Float mu = 0, Float sigma = 1) {
    Float r2 = -2 * std::log1p(-u[0]);
    return {mu + sigma * std::sqrt(r2) * std::cos(2 * PI * u[1]),
            mu + sigma * std::sqrt(r2) * std::sin(2 * PI * u[1])};
  }

  SPECULA_CPU_GPU inline Float logistic_pdf(Float x, Float s) {
    x = std::abs(x);
    return std::exp(-x / s) / (s * sqr(1 + std::exp(-x / s)));
  }

  SPECULA_CPU_GPU inline Float sample_logistic(Float u, Float s) {
    return -s * std::log(1 / u - 1);
  }

  SPECULA_CPU_GPU inline Float invert_logistic_sample(Float x, Float s) {
    return 1 / (1 + std::exp(-x / s));
  }

  SPECULA_CPU_GPU inline Float trimmed_logistic_pdf(Float x, Float s, Float a, Float b) {
    if (x < a || x > b) {
      return 0;
    }

    return logistic(x, s) / (invert_logistic_sample(b, s) - invert_logistic_sample(a, s));
  }

  SPECULA_CPU_GPU inline Float sample_trimmed_logistic(Float u, Float s, Float a, Float b) {
    DASSERT_LT(a, b);
    u = lerp(u, invert_logistic_sample(a, s), invert_logistic_sample(b, s));
    Float x = sample_logistic(u, s);
    DASSERT(!isnan(x));
    return clamp(x, a, b);
  }

  SPECULA_CPU_GPU inline Float invert_trimmed_logistic_sample(Float x, Float s, Float a, Float b) {
    DASSERT(a <= x && x <= b);
    return (invert_logistic_sample(x, s) - invert_logistic_sample(a, s)) /
           (invert_logistic_sample(b, s) - invert_logistic_sample(a, s));
  }

  SPECULA_CPU_GPU inline Float smooth_step_pdf(Float x, Float a, Float b) {
    if (x < a || x > b) {
      return 0;
    }
    DASSERT_LT(a, b);
    return (2 / (b - a)) * smooth_step(x, a, b);
  }

  SPECULA_CPU_GPU inline Float sample_smooth_step(Float u, Float a, Float b) {
    DASSERT_LT(a, b);
    auto cdf_minus_u = [=](Float x) -> std::pair<Float, Float> {
      Float t = (x - a) / (b - a);
      Float p = 2 * pow<3>(t) - pow<4>(t);
      Float pderiv = smooth_step_pdf(x, a, b);
      return {p - u, pderiv};
    };
    return newton_bisection(a, b, cdf_minus_u);
  }

  SPECULA_CPU_GPU inline Float invert_smooth_step_sample(Float x, Float a, Float b) {
    Float t = (x - a) / (b - a);
    auto func = [&](Float x) { return 2 * pow<3>(t) - pow<4>(t); };
    return (func(x) - func(a)) / (func(b) - func(a));
  }

  SPECULA_CPU_GPU inline Point2f sample_uniform_disk_polar(Point2f u) {
    Float r = std::sqrt(u[0]);
    Float theta = 2 * PI * u[1];
    return {r * std::cos(theta), r * std::sin(theta)};
  }

  SPECULA_CPU_GPU inline Point2f invert_uniform_disk_polar_sample(Point2f p) {
    Float phi = std::atan2(p.y, p.x);
    if (phi < 0) {
      phi += 2 * PI;
    }
    return {sqr(p.x) + sqr(p.y), phi / (2 * PI)};
  }

  SPECULA_CPU_GPU inline Point2f sample_uniform_disk_concentric(Point2f u) {
    Point2f uoffset = 2 * u - Vector2f(1, 1);
    if (uoffset.x == 0 && uoffset.y == 0) {
      return {0, 0};
    }

    Float theta = NAN, r = NAN;
    if (std::abs(uoffset.x) > std::abs(uoffset.y)) {
      r = uoffset.x;
      theta = PI_OVER_4 * (uoffset.y / uoffset.x);
    } else {
      r = uoffset.y;
      theta = PI_OVER_2 - PI_OVER_4 * (uoffset.x / uoffset.y);
    }
    return r * Point2f(std::cos(theta), std::sin(theta));
  }

  SPECULA_CPU_GPU inline Point2f invert_uniform_disk_concenctric_sample(Point2f p) {
    Float theta = std::atan2(p.y, p.x);
    Float r = std::sqrt(sqr(p.x) + sqr(p.y));

    Point2f uo;
    if (std::abs(theta) < PI_OVER_4 || std::abs(theta) > 3 * PI_OVER_4) {
      uo.x = r = pstd::copysign(r, p.x);
      if (p.x < 0) {
        if (p.y < 0) {
          uo.y = (PI + theta) * r / PI_OVER_4;
        } else {
          uo.y = (theta - PI) * r / PI_OVER_4;
        }
      } else {
        uo.y = (theta * r) / PI_OVER_4;
      }
    } else {
      uo.y = r = std::copysign(r, p.y);
      if (p.y < 0) {
        uo.x = -(PI_OVER_2 + theta) * r / PI_OVER_4;
      } else {
        uo.x = (PI_OVER_2 - theta) * r / PI_OVER_4;
      }
    }

    return {(uo.x + 1) / 2, (uo.y + 1) / 2};
  }

  SPECULA_CPU_GPU inline Vector3f sample_uniform_hemisphere(Point2f u) {
    Float z = u[0];
    Float r = safe_sqrt(1 - sqr(z));
    Float phi = 2 * PI * u[1];
    return {r * std::cos(phi), r * std::sin(phi), z};
  }

  SPECULA_CPU_GPU inline Float uniform_hemisphere_pdf() { return INV_2PI; }

  SPECULA_CPU_GPU inline Point2f invert_uniform_hemisphere_sample(Vector3f w) {
    Float phi = std::atan2(w.y, w.x);
    if (phi < 0) {
      phi += 2 * PI;
    }
    return {w.z, phi / (2 * PI)};
  }

  SPECULA_CPU_GPU inline Vector3f sample_uniform_sphere(Point2f u) {
    Float z = 1 - 2 * u[0];
    Float r = safe_sqrt(1 - sqr(z));
    Float phi = 2 * PI * u[1];
    return {r * std::cos(phi), r * std::sin(phi), z};
  }

  SPECULA_CPU_GPU inline Float uniform_sphere_pdf() { return INV_4PI; }

  SPECULA_CPU_GPU inline Point2f invert_uniform_sphere_sample(Vector3f w) {
    Float phi = std::atan2(w.y, w.x);
    if (phi < 0) {
      phi += 2 * PI;
    }
    return {(1 - w.z) / 2, phi / (2 * PI)};
  }

  SPECULA_CPU_GPU inline Vector3f sample_cosine_hemisphere(Point2f u) {
    Point2f d = sample_uniform_disk_concentric(u);
    Float z = safe_sqrt(1 - sqr(d.x) - sqr(d.y));
    return {d.x, d.y, z};
  }

  SPECULA_CPU_GPU inline Float cosine_hemisphere_pdf(Float cos_theta) { return cos_theta * INV_PI; }

  SPECULA_CPU_GPU inline Point2f invert_cosine_hemisphere_sample(Vector3f w) {
    return invert_uniform_disk_concenctric_sample({w.x, w.y});
  }

  SPECULA_CPU_GPU inline Float uniform_code_pdf(Float cos_theta_max) {
    return 1 / (2 * PI * (1 - cos_theta_max));
  }

  SPECULA_CPU_GPU inline Vector3f sample_uniform_cone(Point2f u, Float cos_theta_max) {
    Float cos_theta = (1 - u[0]) + u[0] * cos_theta_max;
    Float sin_theta = safe_sqrt(1 - sqr(cos_theta));
    Float phi = u[1] * 2 * PI;
    return spherical_direction(sin_theta, cos_theta, phi);
  }

  SPECULA_CPU_GPU inline Point2f invert_uniform_cone_sample(Vector3f w, Float cos_theta_max) {
    Float cos_theta = w.z;
    Float phi = spherical_phi(w);
    return {(cos_theta - 1) / (cos_theta_max - 1), phi / (2 * PI)};
  }

  SPECULA_CPU_GPU inline Float sample_trimmed_exponential(Float u, Float c, Float xmax) {
    return std::log(1 - u * (1 - std::exp(-c * xmax))) / c;
  }

  SPECULA_CPU_GPU inline Float trimmed_exponential_pdf(Float x, Float c, Float xmax) {
    if (x < 0 || x > xmax) {
      return 0;
    }
    return c / (1 - std::exp(-c * xmax)) * std::exp(-c * x);
  }

  SPECULA_CPU_GPU inline Float invert_trimmed_exponential_sample(Float x, Float c, Float xmax) {
    DASSERT(x >= 0 && x <= xmax);
    return (1 - std::exp(-c * x)) / (1 - std::exp(-c * xmax));
  }

  SPECULA_CPU_GPU inline Vector3f sample_uniform_hemisphere_concentric(Point2f u) {
    Point2f uoffset = 2.0f * u - Vector2f(1, 1);

    if (uoffset.x == 0 && uoffset.y == 0) {
      return {0, 0, 1};
    }

    Float theta = NAN, r = NAN;
    if (std::abs(uoffset.x) > std::abs(uoffset.y)) {
      r = uoffset.x;
      theta = PI_OVER_4 * (uoffset.y / uoffset.x);
    } else {
      r = uoffset.y;
      theta = PI_OVER_2 - PI_OVER_4 * (uoffset.x / uoffset.y);
    }

    return {std::cos(theta) * r * std::sqrt(2 - r * r), std::sin(theta) * r * std::sqrt(2 - r * r),
            1 - r * r};
  }

  SPECULA_CPU_GPU pstd::array<Float, 3> sample_spherical_triangle(const pstd::array<Point3f, 3> &v,
                                                                  Point3f p, Point3f u,
                                                                  Float *pdf = nullptr);
  SPECULA_CPU_GPU Point2f invert_spherical_triangle_sample(const pstd::array<Point3f, 3> &v,
                                                           Point3f p, Vector3f w);

  SPECULA_CPU_GPU Point3f sample_spherical_rectangle(Point3f p, Point3f v00, Vector3f eu,
                                                     Vector3f ev, Point3f u, Float *pdr = nullptr);
  SPECULA_CPU_GPU Point2f invert_spherical_rectangle_sample(Point3f pref, Point3f v00, Vector3f eu,
                                                            Vector3f ev, Point3f prect);

  SPECULA_CPU_GPU Vector3f sample_henyey_greenstein(Vector3f wo, Float g, Point3f u,
                                                    Float *pdf = nullptr);

  SPECULA_CPU_GPU Float sample_catmull_rom(pstd::span<const Float> nodes, pstd::span<const Float> f,
                                           pstd::span<const Float> cdf, Float sample,
                                           Float *fval = nullptr, Float *pdf = nullptr);

  SPECULA_CPU_GPU Float sample_catmull_rom2d(pstd::span<const Float> nodes1,
                                             pstd::span<const Float> nodes2,
                                             pstd::span<const Float> values,
                                             pstd::span<const Float> cdf, Float alpha, Float sample,
                                             Float *fval = nullptr, Float *pdf = nullptr);

  pstd::vector<Float> sample_1d_function(std::function<Float(Float)> func, int n_steps,
                                         int n_samples, Float min = 0, Float max = 1,
                                         Allocator alloc = {});
  Array2D<Float> sample_2d_function(std::function<Float(Float, Float)> func, int nu, int nv,
                                    int n_samples, Bounds2f domain = {Point2f(0, 0), Point2f(1, 1)},
                                    Allocator alloc = {});
} // namespace specula

#endif // SPECULA_UTIL_SAMPLING_FUNCTIONS_HPP
