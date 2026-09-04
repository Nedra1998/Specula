#ifndef SPECULA_UTIL_VECMATH_TUPLE3_HPP
#define SPECULA_UTIL_VECMATH_TUPLE3_HPP

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath/tuple_length.hpp"

namespace specula {
  template <typename T> class Point3;
  template <typename T> class Normal3;

  template <template <typename> class Child, typename T> class Tuple3 {
  public:
    static const int N_DIMENSIONS = 3;

    Tuple3() = default;
    SPECULA_CPU_GPU Tuple3(T x, T y, T z) : x(x), y(y), z(z) { DASSERT(!has_nan()); }

#ifdef SPECULA_DEBUG_BUILD
    SPECULA_CPU_GPU Tuple3(Child<T> c) {
      DASSERT(!c.has_nan());
      x = c.x;
      y = c.y;
      z = c.z;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
    SPECULA_CPU_GPU Child<T> &operator=(Child<T> &c) {
      DASSERRT(!c.has_nan());
      x = c.x;
      y = c.y;
      z = c.z;
      return static_cast<Child<T> &>(*this);
    }
#endif

    SPECULA_CPU_GPU [[nodiscard]] bool has_nan() const { return isnan(x) || isnan(y) || isnan(z); }

    SPECULA_CPU_GPU bool operator==(Child<T> c) const { return x == c.x && y == c.y && z == c.z; }
    SPECULA_CPU_GPU bool operator!=(Child<T> c) const { return x != c.x || y != c.y || z != c.z; }

    template <typename U>
    SPECULA_CPU_GPU auto operator+(Child<U> c) const -> Child<decltype(T{} + U{})> {
      DASSERT(!c.has_nan());
      return {x + c.x, y + c.y, z + c.z};
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator+=(Child<U> c) {
      DASSERT(!c.has_nan());
      x += c.x;
      y += c.y;
      z += c.z;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Child<U> c) const -> Child<decltype(T{} - U{})> {
      DASSERT(!c.has_nan());
      return {x - c.x, y - c.y, z - c.z};
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator-=(Child<U> c) {
      DASSERT(!c.has_nan());
      x -= c.x;
      y -= c.y;
      z -= c.z;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U> SPECULA_CPU_GPU auto operator*(U s) const -> Child<decltype(T{} * U{})> {
      return {s * x, s * y, s * z};
    }

    template <typename U> SPECULA_CPU_GPU auto operator/(U d) const -> Child<decltype(T{} / U{})> {
      DASSERT(d != 0 && !isnan(d));
      return {x / d, y / d, z / d};
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

    SPECULA_CPU_GPU Child<T> operator-() const { return {-x, -y, -z}; }

    SPECULA_CPU_GPU T operator[](int i) const {
      DASSERT(i >= 0 && i <= 2);
      if (i == 0) {
        return x;
      }
      if (i == 1) {
        return y;
      }
      return z;
    }

    SPECULA_CPU_GPU T &operator[](int i) {
      DASSERT(i >= 0 && i <= 2);
      if (i == 0) {
        return x;
      }
      if (i == 1) {
        return y;
      }
      return z;
    }

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

  using Vector3f = Vector3<Float>;
  using Vector3i = Vector3<int>;

  class Vector3fi : public Vector3<Interval> {
  public:
    using Vector3<Interval>::x;
    using Vector3<Interval>::y;
    using Vector3<Interval>::z;
    using Vector3<Interval>::has_nan;
    using Vector3<Interval>::operator+;
    using Vector3<Interval>::operator+=;
    using Vector3<Interval>::operator*;
    using Vector3<Interval>::operator*=;

    Vector3fi() = default;
    SPECULA_CPU_GPU Vector3fi(Float x, Float y, Float z)
        : Vector3<Interval>(Interval(x), Interval(y), Interval(z)) {}
    SPECULA_CPU_GPU Vector3fi(Interval x, Interval y, Interval z) : Vector3<Interval>(x, y, z) {}
    SPECULA_CPU_GPU Vector3fi(Vector3f v)
        : Vector3<Interval>(Interval(v.x), Interval(v.y), Interval(v.z)) {}
    template <typename T>
    SPECULA_CPU_GPU explicit Vector3fi(Point3<T> p)
        : Vector3<Interval>(Interval(p.x), Interval(p.y), Interval(p.z)) {}
    SPECULA_CPU_GPU Vector3fi(Vector3<Interval> vfi) : Vector3<Interval>(vfi) {}

    SPECULA_CPU_GPU Vector3fi(Vector3f v, Vector3f e)
        : Vector3<Interval>(Interval::from_value_and_error(v.x, e.x),
                            Interval::from_value_and_error(v.y, e.y),
                            Interval::from_value_and_error(v.z, e.z)) {}

    SPECULA_CPU_GPU [[nodiscard]] Vector3f error() const {
      return {x.width() / 2, y.width() / 2, z.width() / 2};
    }
    SPECULA_CPU_GPU [[nodiscard]] bool is_exact() const {
      return x.width() == 0 && y.width() == 0 && z.width() == 0;
    }
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

    SPECULA_CPU_GPU Point3() = default;
    SPECULA_GPU Point3(T x, T y, T z) : Tuple3<Point3, T>(x, y, z) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Point3(Point3<U> p) : Tuple3<Point3, T>(T(p.x), T(p.y), T(p.z)) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Point3(Vector3<U> v) : Tuple3<Point3, T>(T(v.x), T(v.y), T(v.z)) {}

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

    SPECULA_CPU_GPU Point3<T> operator-() const { return {-x, -y, -z}; }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Point3<U> p) const -> Vector3<decltype(T{} - U{})> {
      DASSERT(!p.has_nan());
      return {x - p.x, y - p.y, z - p.z};
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
  };

  using Point3f = Point3<Float>;
  using Point3i = Point3<int>;

  class Point3fi : public Point3<Interval> {
  public:
    using Point3<Interval>::x;
    using Point3<Interval>::y;
    using Point3<Interval>::z;
    using Point3<Interval>::has_nan;
    using Point3<Interval>::operator+;
    using Point3<Interval>::operator+=;
    using Point3<Interval>::operator-;
    using Point3<Interval>::operator-=;
    using Point3<Interval>::operator*;
    using Point3<Interval>::operator*=;

    Point3fi() = default;
    SPECULA_CPU_GPU Point3fi(Interval x, Interval y, Interval z) : Point3<Interval>(x, y, z) {}
    SPECULA_CPU_GPU Point3fi(Float x, Float y, Float z)
        : Point3<Interval>(Interval(x), Interval(y), Interval(z)) {}
    SPECULA_CPU_GPU Point3fi(const Point3f &p)
        : Point3<Interval>(Interval(p.x), Interval(p.y), Interval(p.z)) {}
    SPECULA_CPU_GPU Point3fi(Point3<Interval> p) : Point3<Interval>(p) {}
    SPECULA_CPU_GPU Point3fi(Point3f p, Vector3f e)
        : Point3<Interval>(Interval::from_value_and_error(p.x, e.x),
                           Interval::from_value_and_error(p.y, e.y),
                           Interval::from_value_and_error(p.z, e.z)) {}

    SPECULA_CPU_GPU [[nodiscard]] Vector3f error() const {
      return {x.width() / 2, y.width() / 2, z.width() / 2};
    }
    SPECULA_CPU_GPU [[nodiscard]] bool is_exact() const {
      return x.width() == 0 && y.width() == 0 && z.width() == 0;
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
    SPECULA_CPU_GPU explicit Normal3(Normal3<U> n) : Tuple3<Normal3, T>(T(n.x), T(n.y), T(n.z)) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Normal3(Vector3<U> v) : Tuple3<Normal3, T>(T(v.x), T(v.y), T(v.z)) {}
  };

  using Normal3f = Normal3<Float>;

  template <template <typename> class Child, typename T, typename U>
  SPECULA_CPU_GPU inline auto operator*(U s, Tuple3<Child, T> t) -> Child<decltype(T{} * U{})> {
    DASSERT(!t.has_nan());
    return t * s;
  }

  template <template <typename> class Child, typename T> auto format_as(Tuple3<Child, T> t) {
    return std::array<T, 3>{t.x, t.y, t.z};
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
    return {fma(a, b.x, c.x), fma(a, b.y, c.y), fma(a, b.z, c.z)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> fma(Tuple3<Child, T> a, Tuple3<Child, T> b, Tuple3<Child, T> c) {
    return fma(b, a, c);
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min(Tuple3<Child, T> t0, Tuple3<Child, T> t1) {
    using std::min;
    return {min(t0.x, t1.x), min(t0.y, t1.y), min(t0.z, t1.z)};
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min_component_value(Tuple3<Child, T> t) {
    using std::min;
    return min({t.x, t.y, t.z});
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min_component_index(Tuple3<Child, T> t) {
    return (t.x < t.y) ? ((t.x < t.z) ? 0 : 2) : ((t.y < t.z) ? 1 : 2);
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max(Tuple3<Child, T> t0, Tuple3<Child, T> t1) {
    using std::max;
    return {max(t0.x, t1.x), max(t0.y, t1.y), max(t0.z, t1.z)};
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max_component_value(Tuple3<Child, T> t) {
    using std::max;
    return max({t.x, t.y, t.z});
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max_component_index(Tuple3<Child, T> t) {
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

  template <typename T>
  template <typename U>
  SPECULA_CPU_GPU Vector3<T>::Vector3(Point3<U> p) : Tuple3<Vector3, T>(T(p.x), T(p.y), T(p.z)) {}

  template <typename T>
  template <typename U>
  SPECULA_CPU_GPU Vector3<T>::Vector3(Normal3<U> n) : Tuple3<Vector3, T>(T(n.x), T(n.y), T(n.z)) {}

  template <typename T> SPECULA_CPU_GPU inline Vector3<T> cross(Vector3<T> v1, Vector3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return {difference_of_products(v1.y, v2.z, v1.z, v2.y),
            difference_of_products(v1.z, v2.x, v1.x, v2.z),
            difference_of_products(v1.x, v2.y, v1.y, v2.x)};
  }

  template <typename T> SPECULA_CPU_GPU inline Vector3<T> cross(Vector3<T> v1, Normal3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return {difference_of_products(v1.y, v2.z, v1.z, v2.y),
            difference_of_products(v1.z, v2.x, v1.x, v2.z),
            difference_of_products(v1.x, v2.y, v1.y, v2.x)};
  }

  template <typename T> SPECULA_CPU_GPU inline Vector3<T> cross(Normal3<T> v1, Vector3<T> v2) {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return {difference_of_products(v1.y, v2.z, v1.z, v2.y),
            difference_of_products(v1.z, v2.x, v1.x, v2.z),
            difference_of_products(v1.x, v2.y, v1.y, v2.x)};
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

  template <typename T>
  SPECULA_CPU_GPU inline auto dot(Vector3<T> v1, Vector3<T> v2) -> typename TupleLength<T>::type {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Vector3<T> v1, Vector3<T> v2) ->
      typename TupleLength<T>::type {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return std::abs(dot(v1, v2));
  }

  template <typename T> SPECULA_CPU_GPU inline Float angle_between(Vector3<T> v1, Vector3<T> v2) {
    if (dot(v1, v2) < 0) {
      return PI - 2 * safe_asin(length(v1 + v2) / 2);
    } else {
      return 2 * safe_asin(length(v2 - v1) / 2);
    }
  }

  template <typename T> SPECULA_CPU_GPU inline Float angle_between(Normal3<T> n1, Normal3<T> n2) {
    if (dot(n1, n2) < 0) {
      return PI - 2 * safe_asin(length(n1 + n2) / 2);
    } else {
      return 2 * safe_asin(length(n2 - n1) / 2);
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
    *v2 = Vector3<T>(b, sign + sqr(v1.y) * a, -v1.y);
  }

  template <typename T>
  SPECULA_CPU_GPU inline void coordinate_system(Normal3<T> v1, Vector3<T> *v2, Vector3<T> *v3) {
    Float sign = pstd::copysign(Float(1), v1.z);
    Float a = -1 / (sign + v1.z);
    Float b = v1.x * v1.y * a;
    *v2 = Vector3<T>(1 + sign * sqr(v1.x) * a, sign * b, -sign * v1.x);
    *v2 = Vector3<T>(b, sign + sqr(v1.y) * a, -v1.y);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto distance(Point3<T> p1, Point3<T> p2) ->
      typename TupleLength<T>::type {
    return length(p1 - p2);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto distance_squared(Point3<T> p1, Point3<T> p2) ->
      typename TupleLength<T>::type {
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
    DASSERT(!n1.has_nan() && !n2.has_nan());
    return fma(n1.x, n2.x, sum_of_products(n1.y, n2.y, n1.z, n2.z));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Normal3<T> n, Vector3<T> v) -> typename TupleLength<T>::type {
    DASSERT(!n.has_nan() && !v.has_nan());
    return std::abs(dot(n, v));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Vector3<T> v, Normal3<T> n) -> typename TupleLength<T>::type {
    DASSERT(!v.has_nan() && !n.has_nan());
    return std::abs(dot(v, n));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Normal3<T> n1, Normal3<T> n2) ->
      typename TupleLength<T>::type {
    DASSERT(!n1.has_nan() && !n2.has_nan());
    return std::abs(dot(n1, n2));
  }

  template <typename T> SPECULA_CPU_GPU inline Normal3<T> face_forward(Normal3<T> n, Vector3<T> v) {
    return (dot(n, v) < 0.f) ? -n : n;
  }
  template <typename T>
  SPECULA_CPU_GPU inline Normal3<T> face_forward(Normal3<T> n1, Normal3<T> n2) {
    return (dot(n1, n2) < 0.f) ? -n1 : n1;
  }
  template <typename T>
  SPECULA_CPU_GPU inline Vector3<T> face_forward(Vector3<T> v1, Vector3<T> v2) {
    return (dot(v1, v2) < 0.f) ? -v1 : v1;
  }
  template <typename T> SPECULA_CPU_GPU inline Vector3<T> face_forward(Vector3<T> v, Normal3<T> n) {
    return (dot(v, n) < 0.f) ? -v : v;
  }

} // namespace specula

#endif // SPECULA_UTIL_VECMATH_TUPLE3_HPP
