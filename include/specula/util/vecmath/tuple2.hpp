#ifndef INCLUDE_VECMATH_TUPLE2_HPP_
#define INCLUDE_VECMATH_TUPLE2_HPP_

#include <specula/specula.hpp>
#include <specula/util/check.hpp>
#include <specula/util/pstd.hpp>

namespace specula {
  template <typename T> class Point2;
  template <typename T> class Vector2;

  template <template <typename> class Child, typename T> class Tuple2 {
  public:
    static const int N = 2;

    Tuple2() = default;
    SPECULA_CPU_GPU Tuple2(T x, T y) : x(x), y(y) { DASSERT(!has_nan()); }

#ifdef SPECULA_DEBUG_BUILD
    SPECULA_CPU_GPU Tuple2(Child<T> c) {
      DASSERT(!c.has_nan());
      x = c.x;
      y = x.y;
    }

    SPECULA_CPU_GPU Child<T> &operator=(const Child<T> &c) {
      DASSERT(!c.has_nan());
      x = c.x;
      y = c.y;
      return static_cast<Child<T> &>(*this);
    }
#endif

    constexpr size_t size() const { return N; }

    template <typename U>
    SPECULA_CPU_GPU auto operator+(Child<U> c) const -> Child<decltype(T{} + U{})> {
      DASSERT(!c.has_nan());
      return {x + c.x, y + c.y};
    }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Child<U> c) const -> Child<decltype(T{} - U{})> {
      DASSERT(!c.has_nan());
      return {x - c.x, y - c.y};
    }

    template <typename U> SPECULA_CPU_GPU auto operator*(U s) const -> Child<decltype(T{} * U{})> {
      return {s * x, s * y};
    }

    template <typename U> SPECULA_CPU_GPU auto operator/(U d) const -> Child<decltype(T{} / U{})> {
      return {x / d, y / d};
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator+=(Child<U> c) {
      DASSERT(!c.has_nan());
      x += c.x;
      y += c.y;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator-=(Child<U> c) {
      DASSERT(!c.has_nan());
      x -= c.x;
      y -= c.y;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator*=(U s) {
      DASSERT(!isnan(s));
      x *= s;
      y *= s;
      return static_cast<Child<T> &>(*this);
    }

    template <typename U> SPECULA_CPU_GPU Child<T> &operator/=(U d) {
      DASSERT_NE(d, 0);
      DASSERT(!isnan(d));
      x /= d;
      y /= d;
      return static_cast<Child<T> &>(*this);
    }

    SPECULA_CPU_GPU bool operator==(Child<T> c) const { return x == c.x && y == c.y; }
    SPECULA_CPU_GPU bool operator!=(Child<T> c) const { return x != c.x || y != c.y; }

    SPECULA_CPU_GPU Child<T> operator-() const { return {-x, -y}; }

    SPECULA_CPU_GPU T operator[](size_t i) const {
      DASSERT(i >= 0 && i <= 1);
      return i == 0 ? x : y;
    }

    SPECULA_CPU_GPU T &operator[](size_t i) {
      DASSERT(i >= 0 && i <= 1);
      return i == 0 ? x : y;
    }

    SPECULA_CPU_GPU bool has_nan() const { return isnan(x) || isnan(y); }

    T x{}, y{};
  };

  template <typename T> class Vector2 : public Tuple2<Vector2, T> {
  public:
    using Tuple2<Vector2, T>::x;
    using Tuple2<Vector2, T>::y;

    Vector2() = default;
    SPECULA_CPU_GPU Vector2(T x, T y) : Tuple2<Vector2, T>(x, y) {}

    template <typename U> SPECULA_CPU_GPU explicit Vector2(const Point2<U> &p);

    template <typename U>
    SPECULA_CPU_GPU explicit Vector2(const Vector2<U> &v) : Tuple2<Vector2, T>(T(v.x), T(v.y)) {}
  };

  template <typename T> class Point2 : public Tuple2<Point2, T> {
  public:
    using Tuple2<Point2, T>::x;
    using Tuple2<Point2, T>::y;
    using Tuple2<Point2, T>::has_nan;
    using Tuple2<Point2, T>::operator+;
    using Tuple2<Point2, T>::operator+=;
    using Tuple2<Point2, T>::operator*;
    using Tuple2<Point2, T>::operator*=;

    Point2() = default;
    SPECULA_CPU_GPU Point2(T x, T y) : Tuple2<Point2, T>(x, y) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Point2(Point2<U> p) : Tuple2<Point2, T>(T{p.x}, T{p.y}) {}

    template <typename U>
    SPECULA_CPU_GPU explicit Point2(Vector2<U> v) : Tuple2<Point2, T>(T{v.x}, T{v.y}) {}

    template <typename U>
    SPECULA_CPU_GPU auto operator+(Vector2<U> v) const -> Point2<decltype(T{} + U{})> {
      DASSERT(!v.has_nan());
      return {x + v.x, y + v.y};
    }

    template <typename U> SPECULA_CPU_GPU Point2<T> &operator+=(Vector2<U> v) {
      DASSERT(!v.has_nan());
      x += v.x;
      y += v.y;
      return *this;
    }

    SPECULA_CPU_GPU Point2<T> operator-() const { return {-x, -y}; }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Point2<U> p) const -> Vector2<decltype(T{} - U{})> {
      DASSERT(!p.has_nan());
      return {x - p.x, y - p.y};
    }

    template <typename U>
    SPECULA_CPU_GPU auto operator-(Vector2<U> v) const -> Point2<decltype(T{} - U{})> {
      DASSERT(!v.has_nan());
      return {x - v.x, y - v.y};
    }

    template <typename U> SPECULA_CPU_GPU Point2<T> &operator-=(Vector2<U> v) {
      DASSERT(!v.has_nan());
      x -= v.x;
      y -= v.y;
      return *this;
    }
  };

  typedef Vector2<Float> Vector2f;
  typedef Vector2<int> Vector2i;

  typedef Point2<Float> Point2f;
  typedef Point2<int> Point2i;

  template <typename T>
  template <typename U>
  Vector2<T>::Vector2(const Point2<U> &p) : Tuple2<Vector2, T>(T(p.x), T(p.y)) {}

  template <template <class> class Child, typename T, typename U>
  SPECULA_CPU_GPU inline auto operator*(U s, const Tuple2<Child, T> &t)
      -> Tuple2<Child, decltype(U{} * T{})> {
    DASSERT(!t.has_nan());
    return t * s;
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> abs(Tuple2<Child, T> t) {
    using std::abs;
    return {abs(t.x), abs(t.y)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> ceil(Tuple2<Child, T> t) {
    using pstd::ceil;
    return {ceil(t.x), ceil(t.y)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> floor(Tuple2<Child, T> t) {
    using pstd::floor;
    return {floor(t.x), floor(t.y)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline auto lerp(Float t, Tuple2<Child, T> t0, Tuple2<Child, T> t1) {
    return (1 - t) * t0 + t * t1;
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> fma(Float a, Tuple2<Child, T> b, Tuple2<Child, T> c) {
    return {fma(a, b.x, c.x), fma(a, b.y, c.y)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> fma(Tuple2<Child, T> a, Float b, Tuple2<Child, T> c) {
    return fma(b, a, c);
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min(Tuple2<Child, T> a, Tuple2<Child, T> b) {
    using std::min;
    return {min(a.x, b.x), min(a.y, b.y)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline T min_component_value(Tuple2<Child, T> t) {
    using std::min;
    return min({t.x, t.y});
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline size_t min_component_index(Tuple2<Child, T> t) {
    return (t.x < t.y) ? 0 : 1;
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max(Tuple2<Child, T> a, Tuple2<Child, T> b) {
    using std::max;
    return {max(a.x, b.x), max(a.y, b.y)};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline T max_component_value(Tuple2<Child, T> t) {
    using std::max;
    return max({t.x, t.y});
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline size_t max_component_index(Tuple2<Child, T> t) {
    return (t.x > t.y) ? 0 : 1;
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> permute(Tuple2<Child, T> t, pstd::array<int, 2> p) {
    return {t[p[0]], t[p[1]]};
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline T hprod(Tuple2<Child, T> t) {
    return t.x * t.y;
  }

  template <template <typename> class Child, typename T> auto format_as(Tuple2<Child, T> t) {
    return std::array<T, 2>{t.x, t.y};
  }

  template <typename T> auto format_as(Vector2<T> t) { return std::array<T, 2>{t.x, t.y}; }

  template <template <typename> class Child, typename T>
  std::ostream &operator<<(std::ostream &os, const Tuple2<Child, T> &t) {
    return os << fmt::format("{}", t);
  }

} // namespace specula

#endif // INCLUDE_VECMATH_TUPLE2_HPP_
