#ifndef SPECULA_UTIL_SCATTERING_TROWBRIDGE_REITZ_DISTRIBUTION_HPP
#define SPECULA_UTIL_SCATTERING_TROWBRIDGE_REITZ_DISTRIBUTION_HPP

#include <algorithm>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  class TrowbridgeReitzDistribution {
  public:
    TrowbridgeReitzDistribution() = default;

    SPECULA_CPU_GPU TrowbridgeReitzDistribution(Float ax, Float ay) : alpha_x(ax), alpha_y(ay) {
      if (!effectivly_smooth()) {
        alpha_x = std::max<Float>(alpha_x, 1e-4f);
        alpha_y = std::max<Float>(alpha_y, 1e-4f);
      }
    }

    SPECULA_CPU_GPU [[nodiscard]] inline Float d(Vector3f wm) const {
      Float tan2_theta_val = tan2_theta(wm);
      if (isinf(tan2_theta_val)) {
        return 0;
      }
      Float cos4_theta_val = sqr(cos2_theta(wm));
      if (cos4_theta_val < 1e-16f) {
        return 0;
      }
      Float e = tan2_theta_val * (sqr(cos_phi(wm) / alpha_x) + sqr(sin_phi(wm) / alpha_y));
      return 1 / (PI * alpha_x * alpha_y * cos4_theta_val * sqr(1 + e));
    }

    SPECULA_CPU_GPU [[nodiscard]] bool effectivly_smooth() const {
      return std::max(alpha_x, alpha_y) < 1e-3f;
    }

    SPECULA_CPU_GPU [[nodiscard]] Float g1(Vector3f w) const { return 1 / (1 + lambda(w)); }

    SPECULA_CPU_GPU [[nodiscard]] Float lambda(Vector3f w) const {
      Float tan2_theta_val = tan2_theta(w);
      if (isinf(tan2_theta_val)) {
        return 0;
      }
      Float alpha2 = sqr(cos_phi(w) * alpha_x) + sqr(sin_phi(w) * alpha_y);
      return (std::sqrt(1 + alpha2 * tan2_theta_val) - 1) / 2;
    }

    SPECULA_CPU_GPU [[nodiscard]] Float g(Vector3f wo, Vector3f wi) const {
      return 1 / (1 + lambda(wo) + lambda(wi));
    }
    SPECULA_CPU_GPU [[nodiscard]] Float d(Vector3f w, Vector3f wm) const {
      return g1(w) / abs_cos_theta(w) * d(wm) * abs_dot(w, wm);
    }

    SPECULA_CPU_GPU [[nodiscard]] Float pdf(Vector3f w, Vector3f wm) const { return d(w, wm); }

    SPECULA_CPU_GPU [[nodiscard]] Vector3f sample_wm(Vector3f w, Point2f u) const {
      Vector3f wh = normalize(Vector3f(alpha_x * w.x, alpha_y * w.y, w.z));
      if (wh.z < 0) {
        wh = -wh;
      }

      Vector3f t1 = (wh.z < 0.99999f) ? normalize(cross(Vector3f(0, 0, 1), wh)) : Vector3f(1, 0, 0);
      Vector3f t2 = cross(wh, t1);

      // TODO: Uncomment this line once the sampling.hpp header is implemented
      // Point2f p = sample_uniform_disk_polar(u);
      Point2f p;

      Float h = std::sqrt(1 - sqr(p.x));
      p.y = lerp((1 + wh.z) / 2, h, p.y);

      Float pz = std::sqrt(std::max<Float>(0, 1 - length_squared(Vector2f(p))));
      Vector3f nh = p.x * t1 + p.y * t2 + pz * wh;
      ASSERT_RARE(1e-5f, nh.z == 0);
      return normalize(Vector3f(alpha_x * nh.x, alpha_y * nh.y, std::max<Float>(1e-6f, nh.z)));
    }

    SPECULA_CPU_GPU void regularize() {
      if (alpha_x < 0.3f) {
        alpha_x = clamp(2 * alpha_x, 0.1f, 0.3f);
      }
      if (alpha_y < 0.3f) {
        alpha_y = clamp(2 * alpha_y, 0.1f, 0.3f);
      }
    }

    // Note: pbrt-v4 has a bug and instead uses `std::sqrt(roughness)` so the
    // results won't be identical.
    SPECULA_CPU_GPU static Float roughness_to_alpha(Float roughness) { return sqr(roughness); }

  private:
    Float alpha_x, alpha_y;

    friend struct fmt::formatter<TrowbridgeReitzDistribution>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::TrowbridgeReitzDistribution> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::TrowbridgeReitzDistribution &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ TrowbridgeReitzDistribution alpha_x={} alpha_y={} ]", v.alpha_x,
                     v.alpha_y);
  }
};

#endif // SPECULA_UTIL_SCATTERING_TROWBRIDGE_REITZ_DISTRIBUTION_HPP
