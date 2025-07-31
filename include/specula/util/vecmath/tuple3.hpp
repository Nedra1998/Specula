#ifndef INCLUDE_VECMATH_TUPLE3_HPP_
#define INCLUDE_VECMATH_TUPLE3_HPP_

#include <specula/specula.hpp>
#include <specula/util/check.hpp>
#include <specula/util/pstd.hpp>

#include "specula/util/math/constants.hpp"
#include "specula/util/math/functions.hpp"
#include "specula/util/math/interval.hpp"
#include "specula/util/vecmath/tuple2.hpp"

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
    SPECULA_CPU_GPU explicit Vector3(Vector3<U> v) : Tuple3<Vector3, T>(T(v.x), T(v.y), T(v.z)) {}

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
    SPECULA_CPU_GPU explicit Point3(Point3<U> p) : Tuple3<Point3, T>(T(p.x), T(p.y), T(p.z)) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Point3(Vector3<U> v) : Tuple3<Point3, T>(T(v.x), T(v.y), T(v.z)) {}

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
    SPECULA_CPU_GPU auto operator-(Vector3<U> v) const -> Point3<decltype(T{} - U{})> {
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

  class Point3fi : public Point3<Interval> {
  public:
    using Point3<Interval>::x;
    using Point3<Interval>::y;
    using Point3<Interval>::z;
    using Point3<Interval>::has_nan;
    using Point3<Interval>::operator+;
    using Point3<Interval>::operator*;
    using Point3<Interval>::operator*=;

    Point3fi() = default;
    SPECULA_CPU_GPU Point3fi(Interval x, Interval y, Interval z) : Point3<Interval>(x, y, z) {}
    SPECULA_CPU_GPU Point3fi(Float x, Float y, Float z)
        : Point3<Interval>(Interval(x), Interval(y), Interval(z)) {}
    SPECULA_CPU_GPU Point3fi(const Point3<Float> &p)
        : Point3<Interval>(Interval(p.x), Interval(p.y), Interval(p.z)) {}
    SPECULA_CPU_GPU Point3fi(Point3<Interval> p) : Point3<Interval>(p) {}
    SPECULA_CPU_GPU Point3fi(Point3<Float> p, Vector3<Float> e)
        : Point3<Interval>(Interval::from_value_and_error(p.x, e.x),
                           Interval::from_value_and_error(p.y, e.y),
                           Interval::from_value_and_error(p.z, e.z)) {}

    SPECULA_CPU_GPU Vector3<Float> error() const {
      return {x.width() / 2, y.width() / 2, z.width() / 3};
    }
    SPECULA_CPU_GPU bool is_exact() const {
      return x.width() == 0 && y.width() == 0 && z.width() == 0;
    }

    template <typename U> SPECULA_CPU_GPU Point3fi operator+(Vector3<U> v) const {
      DASSERT(!v.has_nan());
      return {x + v.x, y + v.y, z + v.z};
    }

    template <typename U> SPECULA_CPU_GPU Point3fi operator+=(Vector3<U> v) const {
      DASSERT(!v.has_nan());
      x += v.x;
      y += v.y;
      z += v.y;
      return *this;
    }

    template <typename U> SPECULA_CPU_GPU Point3fi operator-() const { return {-x, -y, -z}; }

    template <typename U> SPECULA_CPU_GPU Point3fi operator-(Point3<U> p) const {
      DASSERT(!p.has_nan());
      return {x - p.x, y - p.y, z - p.z};
    }

    template <typename U> SPECULA_CPU_GPU Point3fi operator-(Vector3<U> v) const {
      DASSERT(!v.has_nan());
      return {x - v.x, y - v.y, z - v.z};
    }

    template <typename U> SPECULA_CPU_GPU Point3fi operator-=(Vector3<U> v) const {
      DASSERT(!v.has_nan());
      x -= v.x;
      y -= v.y;
      z -= v.z;
      return *this;
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

  template <typename T>
  template <typename U>
  SPECULA_CPU_GPU Vector3<T>::Vector3(Point3<U> p) : Tuple3<Vector3, T>(T(p.x), T(p.y), T(p.z)) {}

  template <typename T>
  template <typename U>
  SPECULA_CPU_GPU Vector3<T>::Vector3(Normal3<U> n) : Tuple3<Vector3, T>(T(n.x), T(n.y), T(n.z)) {}

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

  template <typename T> SPECULA_CPU_GPU inline Vector3<T> cross(Vector3<T> v1, Vector3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return {
        difference_of_products(v1.y, v2.z, v1.z, v2.y),
        difference_of_products(v1.z, v2.x, v1.x, v2.z),
        difference_of_products(v1.x, v2.y, v1.y, v2.x),
    };
  }

  template <typename T> SPECULA_CPU_GPU inline Vector3<T> cross(Vector3<T> v1, Normal3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return {
        difference_of_products(v1.y, v2.z, v1.z, v2.y),
        difference_of_products(v1.z, v2.x, v1.x, v2.z),
        difference_of_products(v1.x, v2.y, v1.y, v2.x),
    };
  }

  template <typename T> SPECULA_CPU_GPU inline Vector3<T> cross(Normal3<T> v1, Vector3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return {
        difference_of_products(v1.y, v2.z, v1.z, v2.y),
        difference_of_products(v1.z, v2.x, v1.x, v2.z),
        difference_of_products(v1.x, v2.y, v1.y, v2.x),
    };
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto length_squared(Vector3<T> v) -> typename TupleLength<T>::type {
    return sqr(v.x) + sqr(v.y) + sqr(v.z);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto length(Vector3<T> v) -> typename TupleLength<T>::type {
    using std::sqrt;
    return sqrt(length_squared(v));
  }

  template <typename T> SPECULA_CPU_GPU inline auto normalize(Vector3<T> v) {
    return v / length(v);
  }

  template <typename T> SPECULA_CPU_GPU inline T dot(Vector3<T> v1, Vector3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  template <typename T> SPECULA_CPU_GPU inline T abs_dot(Vector3<T> v1, Vector3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return std::abs(dot(v1, v2));
  }

  template <typename T> SPECULA_CPU_GPU inline Float angle_between(Vector2<T> v1, Vector2<T> v2) {
    if (dot(v1, v2) < 0) {
      return Pi - 2 * safe_asin(length(v1 + v2) / 2);
    } else {
      return 2 * safe_asin(length(v2 - v1) / 2);
    }
  }

  template <typename T> SPECULA_CPU_GPU inline Float angle_between(Vector2<T> v1, Normal3<T> v2) {
    if (dot(v1, v2) < 0) {
      return Pi - 2 * safe_asin(length(v1 + v2) / 2);
    } else {
      return 2 * safe_asin(length(v2 - v1) / 2);
    }
  }

  template <typename T>
  SPECULA_CPU_GPU inline Vector3<T> gram_schmidt(Vector3<T> v1, Vector3<T> v2) {
    return v1 - dot(v1, v2) * v2;
  }

  template <typename T>
  SPECULA_CPU_GPU inline void coordinate_system(Vector3<T> v1, Vector3<T> *v2, Vector3<T> *v3) {
    Float sign = pstd::copysign(Float(1), v1.z);
    Float a = -1 / (sign + v1.z);
    Float b = v1.x * v1.y * a;
    *v2 = Vector3<T>(1 + sign * sqr(v1.x) * a, sign * b, -sign * v1.x);
    *v3 + Vector3<T>(b, sign + sqr(v1.y) * a, -v1.y);
  }

  template <typename T>
  SPECULA_CPU_GPU inline void coordinate_system(Normal3<T> v1, Vector3<T> *v2, Vector3<T> *v3) {
    Float sign = pstd::copysign(Float(1), v1.z);
    Float a = -1 / (sign + v1.z);
    Float b = v1.x * v1.y * a;
    *v2 = Vector3<T>(1 + sign * sqr(v1.x) * a, sign * b, -sign * v1.x);
    *v3 + Vector3<T>(b, sign + sqr(v1.y) * a, -v1.y);
  }

  template <typename T> SPECULA_CPU_GPU inline auto distance(Point3<T> p1, Point3<T> p2) {
    return length(p1 - p2);
  }

  template <typename T> SPECULA_CPU_GPU inline auto distance_squared(Point3<T> p1, Point3<T> p2) {
    return length_squared(p1 - p2);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto length_squared(Normal3<T> n) -> typename TupleLength<T>::type {
    return sqr(n.x) + sqr(n.y) + sqr(n.z);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto length(Normal3<T> n) -> typename TupleLength<T>::type {
    using std::sqrt;
    return sqrt(length_squared(n));
  }

  template <typename T> SPECULA_CPU_GPU inline auto normalize(Normal3<T> n) {
    return n / length(n);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto dot(Normal3<T> n, Vector3<T> v) -> typename TupleLength<T>::type {
    DASSERT(!n.has_nan() && !v.has_nan());
    return fma(n.x, v.x, sum_of_products(n.y, v.y, n.z, v.z));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto dot(Vector3<T> v, Normal3<T> n) -> typename TupleLength<T>::type {
    DASSERT(!v.has_nan() && !n.has_nan());
    return fma(v.x, n.x, sum_of_products(v.y, n.y, v.z, n.z));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto dot(Normal3<T> n1, Normal3<T> n2) -> typename TupleLength<T>::type {
    DASSERT(!v.has_nan() && !n.has_nan());
    return fma(n1.x, n2.x, sum_of_products(n1.y, n2.y, n1.z, n2.z));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Normal3<T> n, Vector3<T> v) -> typename TupleLength<T>::type {
    DASSERT(!n.has_nan() && !v.has_nan());
    using std::abs;
    return abs(dot(n, v));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Vector3<T> v, Normal3<T> n) -> typename TupleLength<T>::type {
    DASSERT(!v.has_nan() && !n.has_nan());
    using std::abs;
    return abs(dot(v, n));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Normal3<T> n1, Normal3<T> n2) ->
      typename TupleLength<T>::type {
    DASSERT(!v.has_nan() && !n.has_nan());
    using std::abs;
    return abs(dot(n1, n2));
  }

  template <typename T> SPECULA_CPU_GPU inline auto face_forwards(Normal3<T> n, Vector3<T> v) {
    return (dot(n, v) < 0.0f) ? -n : n;
  }

  template <typename T> SPECULA_CPU_GPU inline auto face_forwards(Normal3<T> n1, Normal3<T> n2) {
    return (dot(n1, n2) < 0.0f) ? -n1 : n2;
  }

  template <typename T> SPECULA_CPU_GPU inline auto face_forwards(Vector3<T> v1, Vector3<T> v2) {
    return (dot(v1, v2) < 0.0f) ? -v1 : v2;
  }

  template <typename T> SPECULA_CPU_GPU inline auto face_forwards(Vector3<T> v, Normal3<T> n) {
    return (dot(v, n) < 0.0f) ? -v : n;
  }

  template <template <typename> class Child, typename T> auto format_as(Tuple3<Child, T> t) {
    return std::array<T, 3>{t.x, t.y, t.z};
  }
} // namespace specula

#endif // INCLUDE_VECMATH_TUPLE3_HPP_
