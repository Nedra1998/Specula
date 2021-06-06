#ifndef SPECULA_UTIL_VECMATH_HPP
#define SPECULA_UTIL_VECMATH_HPP

#include <fmt/format.h>
#include <string>
#include <type_traits>

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

  bool operator==(const Quaternion<T> &rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
  }
  bool operator!=(const Quaternion<T> &rhs) const {
    return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
  }

  std::string to_string() const {
    return fmt::format("[ {}, {}, {}, {} ]", x, y, z, w);
  }

  T x, y, z, w;
};

template <typename T, typename U>
inline std::enable_if_t<std::is_convertible_v<T, U>, Quaternion<U>>
operator*(const T &lhs, const Quaternion<U> &rhs) {
  return rhs * static_cast<U>(lhs);
}

template <typename T>
inline std::ostream &operator<<(std::ostream &out, const Quaternion<T> &rhs) {
  return out << rhs.to_string();
}

using Quaternionf = Quaternion<float>;

} // namespace specula

#endif // SPECULA_UTIL_VECMATH_HPP
