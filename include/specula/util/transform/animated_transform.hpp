#ifndef SPECULA_UTIL_TRANSFORM_ANIMATED_TRANSFORM_HPP
#define SPECULA_UTIL_TRANSFORM_ANIMATED_TRANSFORM_HPP

#include "specula/macros.hpp"
#include "specula/util/math.hpp"
#include "specula/util/transform/transform.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  // TODO: include the ray header once it is implemented
  struct Ray;
  struct RayDifferential;

  struct Interaction;
  struct SurfaceInteraction;

  class AnimatedTransform {
  public:
    AnimatedTransform() = default;
    explicit AnimatedTransform(const Transform &t) : AnimatedTransform(t, 0, t, 1) {}
    AnimatedTransform(const Transform &start, Float tstart, const Transform &end, Float tend);

    SPECULA_CPU_GPU [[nodiscard]] bool is_animated() const { return actually_animated; }
    SPECULA_CPU_GPU [[nodiscard]] bool has_scale() const {
      return start_transform.has_scale() || end_transform.has_scale();
    }

    SPECULA_CPU_GPU [[nodiscard]] Bounds3f motion_bounds(const Bounds3f &b) const;
    SPECULA_CPU_GPU [[nodiscard]] Bounds3f bound_point_motion(Point3f p) const;

    // TODO: once the ray header is implemented implement this functions
    SPECULA_CPU_GPU Ray apply_inverse(const Ray &r, Float *tmax = nullptr) const;

    SPECULA_CPU_GPU [[nodiscard]] Point3f apply_inverse(Point3f p, Float time) const {
      if (!actually_animated) {
        return start_transform.apply_inverse(p);
      }
      return interpolate(time).apply_inverse(p);
    }
    SPECULA_CPU_GPU [[nodiscard]] Vector3f apply_inverse(Vector3f v, Float time) const {
      if (!actually_animated) {
        return start_transform.apply_inverse(v);
      }
      return interpolate(time).apply_inverse(v);
    }
    SPECULA_CPU_GPU [[nodiscard]] Normal3f apply_inverse(Normal3f n, Float time) const {
      if (!actually_animated) {
        return start_transform.apply_inverse(n);
      }
      return interpolate(time).apply_inverse(n);
    }

    // TODO: once the surface header is implemented implement these functions
    SPECULA_CPU_GPU [[nodiscard]] Interaction apply_inverse(const Interaction &it) const;
    SPECULA_CPU_GPU [[nodiscard]] SurfaceInteraction
    apply_inverse(const SurfaceInteraction &it) const;

    // TODO: once the ray header is implemented implement these operators
    SPECULA_CPU_GPU Ray operator()(const Ray &r, Float time) const;
    SPECULA_CPU_GPU RayDifferential operator()(const RayDifferential &r, Float time) const;

    SPECULA_CPU_GPU Point3f operator()(Point3f p, Float time) const;
    SPECULA_CPU_GPU Vector3f operator()(Vector3f v, Float time) const;
    SPECULA_CPU_GPU Normal3f operator()(Normal3f n, Float time) const;

    // TODO: once the surface header is implemented implement these operators
    SPECULA_CPU_GPU Interaction operator()(const Interaction &it) const;
    SPECULA_CPU_GPU SurfaceInteraction operator()(const SurfaceInteraction &it) const;

    SPECULA_CPU_GPU [[nodiscard]] Transform interpolate(Float time) const;

    Transform start_transform, end_transform;
    Float start_time = 0, end_time = 1;

  private:
    struct DerivativeTerm {
      SPECULA_CPU_GPU DerivativeTerm() = default;
      SPECULA_CPU_GPU DerivativeTerm(Float c, Float x, Float y, Float z)
          : kc(c), kx(x), ky(y), kz(z) {}

      SPECULA_CPU_GPU [[nodiscard]] Float eval(Point3f p) const {
        return kc + kx * p.x + ky * p.y + kz * p.z;
      }

      Float kc{}, kx{}, ky{}, kz{};
    };

    SPECULA_CPU_GPU static void find_zeros(Float c1, Float c2, Float c3, Float c4, Float c5,
                                           Float theta, Interval tinterval, pstd::span<Float> zeros,
                                           int *n_zeros, int depth = 8);

    bool actually_animated = false;
    bool has_rotation{};

    Vector3f t[2];
    Quaternion r[2];
    SquareMatrix<4> s[2];
    DerivativeTerm c1[3], c2[3], c3[3], c4[3], c5[3];

    friend struct fmt::formatter<AnimatedTransform>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::AnimatedTransform> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::AnimatedTransform &v, FormatContext &ctx) const {
    return format_to(
        ctx.out(),
        "[ AnimatedTransform startTransform={} endTransform={} startTime={} endTime={} "
        "actuallyAnimated={} T={{{}, {}}} R={{{}, {}}} S={{{}, {}}} hasRotation={}]",
        v.start_transform, v.end_transform, v.start_time, v.end_time, v.actually_animated, v.t[0],
        v.t[1], v.r[0], v.r[1], v.s[0], v.s[1], v.has_rotation);
  }
};

#endif // SPECULA_UTIL_TRANSFORM_ANIMATED_TRANSFORM_HPP
