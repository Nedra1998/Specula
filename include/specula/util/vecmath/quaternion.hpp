#ifndef INCLUDE_VECMATH_QUATERNION_HPP_
#define INCLUDE_VECMATH_QUATERNION_HPP_

#include "specula/util/check.hpp"
#include "specula/util/math/functions.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  class Quaternion {
  public:
    Quaternion() = default;
    SPECULA_CPU_GPU Quaternion(Vector3f v, Float w) : v(v), w(w) {}

    SPECULA_CPU_GPU Quaternion operator+(Quaternion q) const { return {v + q.v, w + q.w}; }
    SPECULA_CPU_GPU Quaternion &operator+=(Quaternion q) {
      v += q.v;
      w += q.w;
      return *this;
    }

    SPECULA_CPU_GPU Quaternion operator-() const { return {-v, -w}; }

    SPECULA_CPU_GPU Quaternion operator-(Quaternion q) const { return {v - q.v, w - q.w}; }
    SPECULA_CPU_GPU Quaternion &operator-=(Quaternion q) {
      v -= q.v;
      w -= q.w;
      return *this;
    }

    SPECULA_CPU_GPU Quaternion operator*(Float f) const { return {v * f, w * f}; }
    SPECULA_CPU_GPU Quaternion &operator*=(Float f) {
      v *= f;
      w *= f;
      return *this;
    }

    SPECULA_CPU_GPU Quaternion operator/(Float f) const {
      DASSERT_NE(0, f);
      return {v / f, w / f};
    }
    SPECULA_CPU_GPU Quaternion &operator/=(Float f) {
      DASSERT_NE(0, f);
      v /= f;
      w /= f;
      return *this;
    }

    Vector3f v;
    Float w = 1;
  };

  SPECULA_CPU_GPU inline Quaternion operator*(Float f, Quaternion q) { return q * f; }

  SPECULA_CPU_GPU inline Float dot(Quaternion q1, Quaternion q2) {
    return dot(q1.v, q2.v) + q1.w * q2.w;
  }

  SPECULA_CPU_GPU inline Float length(Quaternion q) { return std::sqrt(dot(q, q)); }

  SPECULA_CPU_GPU inline Quaternion normalize(Quaternion q) {
    DASSERT_GE(length(q), 0);
    return q / length(q);
  }

  SPECULA_CPU_GPU inline Float angle_between(Quaternion q1, Quaternion q2) {
    if (dot(q1, q2) < 0) {
      return Pi - 2 * safe_asin(length(q1 + q2) / 2);
    } else {
      return 2 * safe_asin(length(q2 - q1) / 2);
    }
  }

  SPECULA_CPU_GPU inline Quaternion slerp(Float t, Quaternion q1, Quaternion q2) {
    Float theta = angle_between(q1, q2);
    Float sin_theta_over_theta = sin_x_over_x(theta);
    return q1 * (1 - t) * sin_x_over_x((1 - t) * theta) / sin_theta_over_theta +
           q2 * t * sin_x_over_x(t * theta) / sin_theta_over_theta;
  }

  inline auto format_as(Quaternion q) { return std::array<Float, 4>{q.v.x, q.v.y, q.v.z, q.w}; }
} // namespace specula

#endif // INCLUDE_VECMATH_QUATERNION_HPP_
