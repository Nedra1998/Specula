#ifndef SPECULA_UTIL_TRANSFORM_TRANSFORM_HPP
#define SPECULA_UTIL_TRANSFORM_TRANSFORM_HPP

#include <limits>

#include "specula/macros.hpp"
#include "specula/util/hash.hpp"
#include "specula/util/math.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  // TODO: include the ray header once it is implemented
  struct Ray;
  struct RayDifferential;

  struct Interaction;
  struct SurfaceInteraction;

  class Transform {
  public:
    Transform() = default;
    SPECULA_CPU_GPU Transform(const SquareMatrix<4> &m) : m(m) {
      pstd::optional<SquareMatrix<4>> inv = inverse(m);
      if (inv) {
        minv = *inv;
      } else {
        Float nan = std::numeric_limits<Float>::has_signaling_NaN
                        ? std::numeric_limits<Float>::signaling_NaN()
                        : std::numeric_limits<Float>::quiet_NaN();
        for (int i = 0; i < 4; ++i) {
          for (int j = 0; j < 4; ++j) {
            minv[i][j] = nan;
          }
        }
      }
    }
    SPECULA_CPU_GPU Transform(const Float mat[4][4]) : Transform(SquareMatrix<4>(mat)) {}
    SPECULA_CPU_GPU Transform(const SquareMatrix<4> &m, const SquareMatrix<4> &inv)
        : m(m), minv(inv) {}
    SPECULA_CPU_GPU explicit Transform(const Frame &frame)
        : Transform(SquareMatrix<4>(frame.x.x, frame.x.y, frame.x.z, 0, frame.y.x, frame.y.y,
                                    frame.y.z, 0, frame.z.x, frame.z.y, frame.z.z, 0, 0, 0, 0, 1)) {

    }
    SPECULA_CPU_GPU explicit Transform(const Quaternion &q) {
      Float xx = q.v.x * q.v.x, yy = q.v.y * q.v.y, zz = q.v.z * q.v.z;
      Float xy = q.v.x * q.v.y, xz = q.v.x * q.v.z, yz = q.v.y * q.v.z;
      Float wx = q.v.x * q.w, wy = q.v.y * q.w, wz = q.v.z * q.w;

      minv[0][0] = 1 - 2 * (yy + zz);
      minv[0][1] = 2 * (xy + wz);
      minv[0][2] = 2 * (xz - wy);

      minv[1][0] = 2 * (xy - wz);
      minv[1][1] = 1 - 2 * (xx + zz);
      minv[1][2] = 2 * (yz + wx);

      minv[2][0] = 2 * (xz + wy);
      minv[2][1] = 2 * (yz - wx);
      minv[2][2] = 1 - 2 * (xx + yy);

      m = transpose(minv);
    }

    SPECULA_CPU_GPU [[nodiscard]] const SquareMatrix<4> &get_matrix() const { return m; }
    SPECULA_CPU_GPU [[nodiscard]] const SquareMatrix<4> &get_inverse_matrix() const { return minv; }

    SPECULA_CPU_GPU bool operator==(const Transform &t) const { return t.m == m; }
    SPECULA_CPU_GPU bool operator!=(const Transform &t) const { return t.m != m; }

    SPECULA_CPU_GPU [[nodiscard]] bool is_identify() const { return m.is_identity(); }
    SPECULA_CPU_GPU [[nodiscard]] bool has_scale(Float tolerance = 1e-3f) const {
      Float la2 = length_squared((*this)(Vector3f(1, 0, 0)));
      Float lb2 = length_squared((*this)(Vector3f(0, 1, 0)));
      Float lc2 = length_squared((*this)(Vector3f(0, 0, 1)));
      return (std::abs(la2 - 1) > tolerance || std::abs(lb2 - 1) > tolerance ||
              std::abs(lc2 - 1) > tolerance);
    }

    SPECULA_CPU_GPU Transform operator*(const Transform &t2) const;
    SPECULA_CPU_GPU [[nodiscard]] bool swap_handedness() const;

    SPECULA_CPU_GPU explicit operator Quaternion() const;

    template <typename T> SPECULA_CPU_GPU Point3<T> operator()(Point3<T> p) const {
      Float xp = m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3];
      Float yp = m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3];
      Float zp = m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3];
      Float wp = m[3][0] * p.x + m[3][1] * p.y + m[3][2] * p.z + m[3][3];

      if (wp == 1) {
        return Point3f(xp, yp, zp);
      } else {
        return Point3f(xp, yp, zp) / wp;
      }
    }

    template <typename T> SPECULA_CPU_GPU Vector3<T> operator()(Vector3<T> v) const {
      return {m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
              m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
              m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z};
    }

    template <typename T> SPECULA_CPU_GPU Normal3<T> operator()(Normal3<T> n) const {
      return {minv[0][0] * n.x + minv[1][0] * n.y + minv[2][0] * n.z,
              minv[0][1] * n.x + minv[1][1] * n.y + minv[2][1] * n.z,
              minv[0][2] * n.x + minv[1][2] * n.y + minv[2][2] * n.z};
    }

    // TODO: Once ray header is implemented implement these transformers
    template <typename T> SPECULA_CPU_GPU Ray operator()(const Ray &r, Float *tmax = nullptr) const;
    template <typename T>
    SPECULA_CPU_GPU RayDifferential operator()(const RayDifferential &r,
                                               Float *tmax = nullptr) const;

    SPECULA_CPU_GPU Bounds3f operator()(const Bounds3f &b) const;

    // TODO: Once surface header is implemented, then implement these operators
    SPECULA_CPU_GPU Interaction operator()(const Interaction &in) const;
    SPECULA_CPU_GPU SurfaceInteraction operator()(const SurfaceInteraction &si) const;

    SPECULA_CPU_GPU Point3fi operator()(const Point3fi &p) const {
      auto x = Float(p.x), y = Float(p.y), z = Float(p.z);
      Float xp = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
      Float yp = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
      Float zp = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
      Float wp = m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3];

      Vector3f perror;
      if (p.is_exact()) {
        perror.x = gamma(3) * (std::abs(m[0][0] * x) + std::abs(m[0][1] * y) +
                               std::abs(m[0][2] * z) + std::abs(m[0][3]));
        perror.y = gamma(3) * (std::abs(m[1][0] * x) + std::abs(m[1][1] * y) +
                               std::abs(m[1][2] * z) + std::abs(m[1][3]));
        perror.z = gamma(3) * (std::abs(m[2][0] * x) + std::abs(m[2][1] * y) +
                               std::abs(m[2][2] * z) + std::abs(m[2][3]));
      } else {
        Vector3f pin_error = p.error();
        perror.x =
            (gamma(3) + 1) * (std::abs(m[0][0]) * pin_error.x + std::abs(m[0][1]) * pin_error.y +
                              std::abs(m[0][2]) * pin_error.z) +
            gamma(3) * (std::abs(m[0][0] * x) + std::abs(m[0][1] * y) + std::abs(m[0][2] * z) +
                        std::abs(m[0][3]));
        perror.y =
            (gamma(3) + 1) * (std::abs(m[1][0]) * pin_error.x + std::abs(m[1][1]) * pin_error.y +
                              std::abs(m[1][2]) * pin_error.z) +
            gamma(3) * (std::abs(m[1][0] * x) + std::abs(m[1][1] * y) + std::abs(m[1][2] * z) +
                        std::abs(m[1][3]));
        perror.z =
            (gamma(3) + 1) * (std::abs(m[2][0]) * pin_error.x + std::abs(m[2][1]) * pin_error.y +
                              std::abs(m[2][2]) * pin_error.z) +
            gamma(3) * (std::abs(m[2][0] * x) + std::abs(m[2][1] * y) + std::abs(m[2][2] * z) +
                        std::abs(m[2][3]));
      }

      if (wp == 1) {
        return {Point3f(xp, yp, zp), perror};
      } else {
        return Point3fi(Point3f(xp, yp, zp), perror) / wp;
      }
    }

    SPECULA_CPU_GPU inline Vector3fi operator()(const Vector3fi &v) const {
      auto x = Float(v.x), y = Float(v.y), z = Float(v.z);
      Float xp = m[0][0] * x + m[0][1] * y + m[0][2] * z;
      Float yp = m[1][0] * x + m[1][1] * y + m[1][2] * z;
      Float zp = m[2][0] * x + m[2][1] * y + m[2][2] * z;

      Vector3f verror;
      if (v.is_exact()) {
        verror.x =
            gamma(3) * (std::abs(m[0][0] * x) + std::abs(m[0][1] * y) + std::abs(m[0][2] * z));
        verror.y =
            gamma(3) * (std::abs(m[1][0] * x) + std::abs(m[1][1] * y) + std::abs(m[1][2] * z));
        verror.z =
            gamma(3) * (std::abs(m[2][0] * x) + std::abs(m[2][1] * y) + std::abs(m[2][2] * z));
      } else {
        Vector3f vin_error = v.error();
        verror.x =
            (gamma(3) + 1) * (std::abs(m[0][0]) * vin_error.x + std::abs(m[0][1]) * vin_error.y +
                              std::abs(m[0][2]) * vin_error.z) +
            gamma(3) * (std::abs(m[0][0] * x) + std::abs(m[0][1] * y) + std::abs(m[0][2] * z));
        verror.y =
            (gamma(3) + 1) * (std::abs(m[1][0]) * vin_error.x + std::abs(m[1][1]) * vin_error.y +
                              std::abs(m[1][2]) * vin_error.z) +
            gamma(3) * (std::abs(m[1][0] * x) + std::abs(m[1][1] * y) + std::abs(m[1][2] * z));
        verror.z =
            (gamma(3) + 1) * (std::abs(m[2][0]) * vin_error.x + std::abs(m[2][1]) * vin_error.y +
                              std::abs(m[2][2]) * vin_error.z) +
            gamma(3) * (std::abs(m[2][0] * x) + std::abs(m[2][1] * y) + std::abs(m[2][2] * z));
      }

      return {Vector3f(xp, yp, zp), verror};
    }

    template <typename T> SPECULA_CPU_GPU inline Vector3<T> apply_inverse(Vector3<T> v) const {
      return {minv[0][0] * v.x + minv[0][1] * v.y + minv[0][2] * v.z,
              minv[1][0] * v.x + minv[1][1] * v.y + minv[1][2] * v.z,
              minv[2][0] * v.x + minv[2][1] * v.y + minv[2][2] * v.z};
    }
    template <typename T> SPECULA_CPU_GPU inline Normal3<T> apply_inverse(Normal3<T> n) const {
      return {m[0][0] * n.x + m[1][0] * n.y + m[2][0] * n.z,
              m[0][1] * n.x + m[1][1] * n.y + m[2][1] * n.z,
              m[0][2] * n.x + m[1][2] * n.y + m[2][2] * n.z};
    }

    template <typename T> SPECULA_CPU_GPU inline Point3<T> apply_inverse(Point3<T> p) const {
      Float xp = minv[0][0] * p.x + minv[0][1] * p.y + minv[0][2] * p.z + minv[0][3];
      Float yp = minv[1][0] * p.x + minv[1][1] * p.y + minv[1][2] * p.z + minv[1][3];
      Float zp = minv[2][0] * p.x + minv[2][1] * p.y + minv[2][2] * p.z + minv[2][3];
      Float wp = minv[3][0] * p.x + minv[3][1] * p.y + minv[3][2] * p.z + minv[3][3];

      ASSERT_NE(wp, 0);
      if (wp == 1) {
        return Point3f(xp, yp, zp);
      } else {
        return Point3f(xp, yp, zp) / wp;
      }
    }

    SPECULA_CPU_GPU [[nodiscard]] Point3fi apply_inverse(const Point3fi &p) const;

    // TODO: Once ray header is implemented implement these transformers
    SPECULA_CPU_GPU inline Ray apply_inverse(const Ray &r, Float *tmax = nullptr) const;
    SPECULA_CPU_GPU inline RayDifferential apply_inverse(const RayDifferential &r,
                                                         Float *tmax = nullptr) const;

    // TODO: Once surface header is implemented, then implement these operators
    SPECULA_CPU_GPU [[nodiscard]] Interaction apply_inverse(const Interaction &in) const;
    SPECULA_CPU_GPU [[nodiscard]] SurfaceInteraction
    apply_inverse(const SurfaceInteraction &si) const;

    void decompose(Vector3f *t, SquareMatrix<4> *r, SquareMatrix<4> *s) const;

  private:
    SquareMatrix<4> m, minv;

    friend struct fmt::formatter<Transform>;
  };

  SPECULA_CPU_GPU Transform translate(Vector3f delta);
  SPECULA_CPU_GPU Transform scale(Float x, Float y, Float z);
  SPECULA_CPU_GPU Transform rotate_x(Float theta);
  SPECULA_CPU_GPU Transform rotate_y(Float theta);
  SPECULA_CPU_GPU Transform rotate_z(Float theta);

  SPECULA_CPU_GPU Transform look_at(Point3f pos, Point3f look, Vector3f up);
  SPECULA_CPU_GPU Transform orthographic(Float znear, Float zfar);
  SPECULA_CPU_GPU Transform perspective(Float fov, Float znear, Float zfar);

  SPECULA_CPU_GPU inline Transform inverse(const Transform &t) {
    return {t.get_inverse_matrix(), t.get_matrix()};
  }
  SPECULA_CPU_GPU inline Transform transpose(const Transform &t) {
    return {transpose(t.get_matrix()), transpose(t.get_inverse_matrix())};
  }
  SPECULA_CPU_GPU inline Transform rotate(Float sin_theta, Float cos_theta, Vector3f axis) {
    Vector3f a = normalize(axis);
    SquareMatrix<4> m;
    m[0][0] = a.x * a.x + (1 - a.x * a.x) * cos_theta;
    m[0][1] = a.x * a.y * (1 - cos_theta) - a.z * sin_theta;
    m[0][2] = a.x * a.z * (1 - cos_theta) + a.y * sin_theta;
    m[0][3] = 0;

    m[1][0] = a.x * a.y * (1 - cos_theta) + a.z * sin_theta;
    m[1][1] = a.y * a.y + (1 - a.y * a.y) * cos_theta;
    m[1][2] = a.y * a.z * (1 - cos_theta) - a.x * sin_theta;
    m[1][3] = 0;

    m[2][0] = a.x * a.z * (1 - cos_theta) - a.y * sin_theta;
    m[2][1] = a.y * a.z * (1 - cos_theta) + a.x * sin_theta;
    m[2][2] = a.z * a.z + (1 - a.z * a.z) * cos_theta;
    m[2][3] = 0;

    return {m, transpose(m)};
  }

  SPECULA_CPU_GPU inline Transform rotate(Float theta, Vector3f axis) {
    Float sin_theta = std::sin(radians(theta));
    Float cos_theta = std::cos(radians(theta));
    return rotate(sin_theta, cos_theta, axis);
  }

  SPECULA_CPU_GPU inline Transform rotate_from_to(Vector3f from, Vector3f to) {
    Vector3f refl;
    if (std::abs(from.x) < 0.72f && std::abs(to.x) < 0.72f) {
      refl = Vector3f(1, 0, 0);
    } else if (std::abs(from.y) < 0.72f && std::abs(to.y) < 0.72f) {
      refl = Vector3f(0, 1, 0);
    } else {
      refl = Vector3f(0, 0, 1);
    }

    Vector3f u = refl - from, v = refl - to;
    SquareMatrix<4> r;

    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        r[i][j] = ((i == j) ? 1 : 0) - 2 / dot(u, u) * u[i] * u[j] - 2 / dot(v, v) * v[i] * v[j] +
                  4 * dot(u, v) / (dot(u, u) * dot(v, v)) * v[i] * u[j];
      }
    }

    return {r, transpose(r)};
  }
} // namespace specula

template <> struct std::hash<specula::Transform> {
  SPECULA_CPU_GPU size_t operator()(const specula::Transform &t) const {
    specula::SquareMatrix<4> m = t.get_matrix();
    return specula::hash(m);
  }
};

template <> struct fmt::formatter<specula::Transform> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Transform &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ m={} mInv={} ]", v.m, v.minv);
  }
};

#endif // SPECULA_UTIL_TRANSFORM_TRANSFORM_HPP
