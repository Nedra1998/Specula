#ifndef INCLUDE_VECMATH_TUPLE3_HPP_
#define INCLUDE_VECMATH_TUPLE3_HPP_

#include <specula/specula.hpp>
#include <specula/util/check.hpp>
#include <specula/util/pstd.hpp>

namespace specula {
  template <typename T> class Point3;
  template <typename T> class Normal3;
  template <typename T> class Vector3;

  template <template <typename> class Child, typename T> class Tuple3 {
  public:
    constexpr size_t size() const { return 3; }

    Tuple3() = default;
    SPECULA_CPU_GPU Tuple3(T x, T y, T z) : x(x), y(y), z(z) { DASSERT(!has_nan()); }

#ifdef SPECULA_DEBUG_BUILD
    SPECULA_CPU_GPU Tuple3(Child<T> c) {
      DASSERT(!c.has_nan());
      x = c.x;
      y = c.y;
      z = c.z;
    }

    SPECULA_CPU_GPU Child<T> &operator=(const Child<T> &c) {
      DASSERT(!c.has_nan());
      x = c.x;
      y = c.y;
      z = c.z;
      return static_cast<Child<T> &>(*this);
    }
#endif // SPECULA_DEBUG_BUILD

    template <typename U>
    SPECULA_CPU_GPU auto operator+(Child<U> c) const -> Child<decltype(T{} + U{})> {
      DASSERT(!c.has_nan());
      return {x + c.x, y + c.y, z + c.z};
    }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Child<U> c) const -> Child<decltype(T{} - U{})> {
      DASSERT(!c.has_nan());
      return {x - c.x, y - c.y, z - c.z};
    }

    template <typename U> SPECULA_CPU_GPU auto operator*(U s) const -> Child<decltype(T{} * U{})> {
      return {s * x, s * y, s * z};
    }

    template <typename U> SPECULA_CPU_GPU auto operator/(U d) const -> Child<decltype(T{} / U{})> {
      return {x / d, y / d, z / d};
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator+=(Child<U> c) {
      DASSERT(!c.has_nan());
      x += c.x;
      y += c.y;
      z += c.z;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator-=(Child<U> c) {
      DASSERT(!c.has_nan());
      x -= c.x;
      y -= c.y;
      z -= c.z;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator*=(U s) {
      DASSERT(!isnan(s));
      x *= s;
      y *= s;
      z *= s;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator/=(U d) {
      DASSERT_NE(d, 0);
      DASSERT(!isnan(d));
      x /= d;
      y /= d;
      z /= d;
      return static_cast<Child<T> &>(*this);
    }

    SPECULA_CPU_GPU bool operator==(Child<T> c) const { return x == c.x && y == c.y && z == c.z; }
    SPECULA_CPU_GPU bool operator!=(Child<T> c) const { return x != c.x || y != c.y || z != c.z; }

    SPECULA_CPU_GPU Child<T> operator-() const { return {-x, -y, -z}; }

    SPECULA_CPU_GPU T operator[](size_t i) const {
      DASSERT(i >= 0 && i <= 2);
      if (i == 0)
        return x;
      if (i == 1)
        return y;
      return z;
    }

    SPECULA_CPU_GPU T &operator[](size_t i) {
      DASSERT(i >= 0 && i <= 2);
      if (i == 0)
        return x;
      if (i == 1)
        return y;
      return z;
    }

    SPECULA_CPU_GPU bool has_nan() const { return isnan(x) || isnan(y) || isnan(z); }

    T x{}, y{}, z{};
  };

  template <typename T> class Vector3 : public Tuple3<Vector3, T> {
  public:
    using Tuple3<Vector3, T>::x;
    using Tuple3<Vector3, T>::y;
    using Tuple3<Vector3, T>::z;

    Vector3() = default;
    SPECULA_CPU_GPU Vector3(T x, T y, T z) : Tuple3<Vector3, T>(x, y, z) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Vector3(Vector3<U> v) : Tuple3<Vector3, T>(T{v.x}, T{v.y}, T{v.z}) {}

    template <typename U> SPECULA_CPU_GPU explicit Vector3(Point3<U> p);
    template <typename U> SPECULA_CPU_GPU explicit Vector3(Normal3<U> n);
  };

  template <typename T> class Point3 : public Tuple3<Point3, T> {
  public:
    using Tuple3<Point3, T>::x;
    using Tuple3<Point3, T>::y;
    using Tuple3<Point3, T>::z;
    using Tuple3<Point3, T>::has_nan;
    using Tuple3<Point3, T>::operator+;
    using Tuple3<Point3, T>::operator+=;
    using Tuple3<Point3, T>::operator*;
    using Tuple3<Point3, T>::operator*=;

    Point3() = default;
    SPECULA_CPU_GPU Point3(T x, T y, T z) : Tuple3<Point3, T>(x, y, z) {}

    template <typename U>
    SPECULA_CPU_GPU explicit Point3(Point3<U> p) : Tuple3<Point3, T>(T{p.x}, T{p.y}, T{p.z}) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Point3(Vector3<U> v) : Tuple3<Point3, T>(T{v.x}, T{v.y}, T{v.z}) {}

    SPECULA_CPU_GPU Point3<T> operator-() const { return {-x, -y, -z}; }

    template <typename U>
    SPECULA_CPU_GPU auto operator+(Vector3<U> v) const -> Point3<decltype(T{} + U{})> {
      DASSERT(!v.has_nan());
      return {x + v.x, y + v.y, z + v.z};
    }

    template <typename U> SPECULA_CPU_GPU Point3<T> &operator+=(Vector3<U> v) {
      DASSERT(!v.has_nan());
      x += v.x;
      y += v.y;
      z += v.z;
      return *this;
    }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Vector3<U> v) const -> Vector3<decltype(T{} - U{})> {
      DASSERT(!v.has_nan());
      return {x - v.x, y - v.y, z - v.z};
    }

    template <typename U> SPECULA_CPU_GPU Point3<T> &operator-=(Vector3<U> v) {
      DASSERT(!v.has_nan());
      x -= v.x;
      y -= v.y;
      z -= v.z;
      return *this;
    }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Point3<U> p) const -> Vector3<decltype(T{} - U{})> {
      DASSERT(!p.has_nan());
      return {x - p.x, y - p.y, z - p.z};
    }
  };

  template <typename T> class Normal3 : public Tuple3<Normal3, T> {
  public:
    using Tuple3<Normal3, T>::x;
    using Tuple3<Normal3, T>::y;
    using Tuple3<Normal3, T>::z;
    using Tuple3<Normal3, T>::has_nan;
    using Tuple3<Normal3, T>::operator+;
    using Tuple3<Normal3, T>::operator*;
    using Tuple3<Normal3, T>::operator*=;

    Normal3() = default;
    SPECULA_CPU_GPU Normal3(T x, T y, T z) : Tuple3<Normal3, T>(x, y, z) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Normal3(Normal3<U> n) : Tuple3<Normal3, T>(T{n.x}, T{n.y}, T{n.z}) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Normal3(Vector3<U> v) : Tuple3<Normal3, T>(T{v.x}, T{v.y}, T{v.z}) {}
  };

  typedef Vector3<Float> Vector3f;
  typedef Vector3<int> Vector3i;

  typedef Point3<Float> Point3f;
  typedef Point3<int> Point3i;

  typedef Normal3<Float> Normal3f;

  template <template <typename> class Child, typename T, typename U>
  SPECULA_CPU_GPU inline auto operator*(U s, Tuple3<Child, T> t) -> Child<decltype(T{} * U{})> {
    DASSERT(!t.has_nan());
    return t * s;
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> abs(Tuple3<Child, T> t) {
    using std::abs;
    return {abs(t.x), abs(t.y), abs(t.z)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> ceil(Tuple3<Child, T> t) {
    using pstd::ceil;
    return {ceil(t.x), ceil(t.y), ceil(t.z)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> floor(Tuple3<Child, T> t) {
    using pstd::floor;
    return {floor(t.x), floor(t.y), floor(t.z)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline auto lerp(Float t, Tuple3<Child, T> t0, Tuple3<Child, T> t1) {
    return (1 - t) * t0 + t * t1;
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> fma(Float a, Tuple3<Child, T> b, Tuple3<Child, T> c) {
    return {fma(a, b.x, c.x), fma(a, b.y, c.y)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> fma(Tuple3<Child, T> a, Float b, Tuple3<Child, T> c) {
    return fma(b, a, c);
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min(Tuple3<Child, T> a, Tuple3<Child, T> b) {
    using std::min;
    return {min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline T min_component_value(Tuple3<Child, T> t) {
    using std::min;
    return min({t.x, t.y, t.z});
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline size_t min_component_index(Tuple3<Child, T> t) {
    return (t.x < t.y) ? ((t.x < t.z) ? 0 : 2) : ((t.y < t.z) ? 1 : 2);
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max(Tuple3<Child, T> a, Tuple3<Child, T> b) {
    using std::max;
    return {max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline T max_component_value(Tuple3<Child, T> t) {
    using std::max;
    return max({t.x, t.y, t.z});
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline size_t max_component_index(Tuple3<Child, T> t) {
    return (t.x > t.y) ? ((t.x > t.z) ? 0 : 2) : ((t.y > t.z) ? 1 : 2);
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> permute(Tuple3<Child, T> t, pstd::array<int, 3> p) {
    return {t[p[0]], t[p[1]], t[p[2]]};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline T hprod(Tuple3<Child, T> t) {
    return t.x * t.y * t.z;
  }

  template <template <typename> class Child, typename T> auto format_as(Tuple3<Child, T> t) {
    return std::array<T, 3>{t.x, t.y, t.z};
  }

  template <template <typename> class Child, typename T>
  std::ostream &operator<<(std::ostream &os, const Tuple3<Child, T> &t) {
    return os << fmt::format("{}", t);
  }
} // namespace specula

#endif // INCLUDE_VECMATH_TUPLE3_HPP_
