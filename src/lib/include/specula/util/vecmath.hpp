#ifndef SPECULA_UTIL_VECMATH_HPP
#define SPECULA_UTIL_VECMATH_HPP

namespace specula {
template <typename T> class Quaternion {
public:
  Quaternion() : x(), y(), z(), w(1) {}
  Quaternion(const T &x, const T &y, const T &z, const T &w = 1)
      : x(x), y(y), z(z), w(w) {}

  Quaternion &operator+=(const Quaternion &q) {
    x += q.x;
    y += q.y;
    z += q.z;
    w += q.w;
    return *this;
  }
  Quaternion operator+(const Quaternion &q) const {
    return {x + q.x, y + q.y, z + q.z, w + q.w};
  }
  Quaternion &operator-=(const Quaternion &q) {
    x -= q.x;
    y -= q.y;
    z -= q.z;
    w -= q.w;
    return *this;
  }
  Quaternion operator-() const { return {-x, -y, -z, -w}; }
  Quaternion operator-(const Quaternion &q) const {
    return {x - q.x, y - q.y, z - q.z, w - q.w};
  }
  Quaternion &operator*=(const T &f) {
    x *= f;
    y *= f;
    z *= f;
    w *= f;
    return *this;
  }
  Quaternion operator*(const T &f) const {
    return {x * f, y * f, z * f, w * f};
  }
  Quaternion &operator/=(const T &f) {

    x /= f;
    y /= f;
    z /= f;
    w /= f;
    return *this;
  }
  Quaternion operator/(const T &f) const {
    return {x / f, y / f, z / f, w / f};
  }

  T x, y, z, w;
};
} // namespace specula

#endif // SPECULA_UTIL_VECMATH_HPP
