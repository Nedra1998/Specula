#ifndef SPECULA_UTIL_VECMATH_TUPLE2_HPP
#define SPECULA_UTIL_VECMATH_TUPLE2_HPP

#include <fmt/base.h>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath/tuple_length.hpp"

namespace specula {
  template <typename T> class Point2;

  template <template <typename> class Child, typename T> class Tuple2 {
  public:
    static const int N_DIMENSIONS = 2;

    Tuple2() = default;
    SPECULA_CPU_GPU Tuple2(T x, T y) : x(x), y(y) { DASSERT(!has_nan()); }

#ifdef SPECULA_DEBUG_BUILD
    SPECULA_CPU_GPU Tuple2(Child<T> c) {
      DASSERT(!c.has_nan());
      x = c.x;
      y = c.y;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
    SPECULA_CPU_GPU Child<T> &operator=(Child<T> &c) {
      DASSERT(!c.has_nan());
      x = c.x;
      y = c.y;
      return static_cast<Child<T> &>(*this);
    }
#endif

    SPECULA_CPU_GPU [[nodiscard]] bool has_nan() const { return isnan(x) || isnan(y); }

    SPECULA_CPU_GPU bool operator==(Child<T> c) const { return x == c.x && y == c.y; }
    SPECULA_CPU_GPU bool operator!=(Child<T> c) const { return x != c.x || y != c.y; }

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

    template <typename U> SPECULA_CPU_GPU auto operator*(U s) const -> Child<decltype(T{} * U{})> {
      return {s * x, s * y};
    }

    template <typename U> SPECULA_CPU_GPU auto operator/(U d) const -> Child<decltype(T{} / U{})> {
      DASSERT(d != 0 && !isnan(d));
      return {x / d, y / d};
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

    SPECULA_CPU_GPU Child<T> operator-() const { return {-x, -y}; }

    SPECULA_CPU_GPU T operator[](int i) const {
      DASSERT(i >= 0 && i <= 1);
      return (i == 0) ? x : y;
    }
    SPECULA_CPU_GPU T &operator[](int i) {
      DASSERT(i >= 0 && i <= 1);
      return (i == 0) ? x : y;
    }

    T x{}, y{};
  };

  template <typename T> class Vector2 : public Tuple2<Vector2, T> {
  public:
    using Tuple2<Vector2, T>::x;
    using Tuple2<Vector2, T>::y;

    Vector2() = default;
    SPECULA_CPU_GPU Vector2(T x, T y) : Tuple2<Vector2, T>(x, y) {}
    template <typename U> SPECULA_CPU_GPU explicit Vector2(Point2<U> p);
    template <typename U>
    SPECULA_CPU_GPU explicit Vector2(Vector2<U> v) : Tuple2<Vector2, T>(T(v.x), T(v.y)) {}
  };

  using Vector2f = Vector2<Float>;
  using Vector2i = Vector2<int>;

  template <typename T> class Point2 : public Tuple2<Point2, T> {
  public:
    using Tuple2<Point2, T>::x;
    using Tuple2<Point2, T>::y;
    using Tuple2<Point2, T>::has_nan;
    using Tuple2<Point2, T>::operator+;
    using Tuple2<Point2, T>::operator+=;
    using Tuple2<Point2, T>::operator*;
    using Tuple2<Point2, T>::operator*=;

    SPECULA_CPU_GPU Point2() = default;
    SPECULA_CPU_GPU Point2(T x, T y) : Tuple2<Point2, T>(x, y) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Point2(Point2<U> p) : Tuple2<Point2, T>(T(p.x), T(p.y)) {}
    template <typename U>
    SPECULA_CPU_GPU explicit Point2(Vector2<U> v) : Tuple2<Point2, T>(T(v.x), T(v.y)) {}

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

  using Point2f = Point2<Float>;
  using Point2i = Point2<int>;

  template <template <typename> class Child, typename T, typename U>
  SPECULA_CPU_GPU inline auto operator*(U s, Tuple2<Child, T> t) -> Child<decltype(T{} * U{})> {
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
  SPECULA_CPU_GPU inline Child<T> fma(Tuple2<Child, T> a, Tuple2<Child, T> b, Tuple2<Child, T> c) {
    return fma(b, a, c);
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min(Tuple2<Child, T> t0, Tuple2<Child, T> t1) {
    using std::min;
    return {min(t0.x, t1.x), min(t0.y, t1.y)};
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min_component_value(Tuple2<Child, T> t) {
    using std::min;
    return min({t.x, t.y});
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> min_component_index(Tuple2<Child, T> t) {
    return (t.x < t.y) ? 0 : 1;
  }

  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max(Tuple2<Child, T> t0, Tuple2<Child, T> t1) {
    using std::max;
    return {max(t0.x, t1.x), max(t0.y, t1.y)};
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max_component_value(Tuple2<Child, T> t) {
    using std::max;
    return max({t.x, t.y});
  }
  template <template <typename> class Child, typename T>
  SPECULA_CPU_GPU inline Child<T> max_component_index(Tuple2<Child, T> t) {
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

  template <typename T>
  template <typename U>
  SPECULA_CPU_GPU Vector2<T>::Vector2(Point2<U> p) : Tuple2<Vector2, T>(T(p.x), T(p.y)) {}

  template <typename T>
  SPECULA_CPU_GPU inline auto dot(Vector2<T> v1, Vector2<T> v2) -> typename TupleLength<T>::type {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return sum_of_products(v1.x, v2.x, v1.y, v2.y);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto abs_dot(Vector2<T> v1, Vector2<T> v2) ->
      typename TupleLength<T>::type {
    DASSERT(!v1.has_nan() && !v2.has_nan());
    return std::abs(dot(v1, v2));
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto length_squared(Vector2<T> v) -> typename TupleLength<T>::type {
    return sqr(v.x) + sqr(v.y);
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto length(Vector2<T> v) -> typename TupleLength<T>::type {
    using std::sqrt;
    return sqrt(length_squared(v));
  }

  template <typename T> SPECULA_CPU_GPU inline auto normalize(Vector2<T> v) {
    return v / length(v);
  }

  SPECULA_CPU_GPU inline Point2f invert_bilinear(Point2f p, pstd::span<const Point2f> vert) {
    Point2f a = vert[0], b = vert[1], c = vert[3], d = vert[2];
    Vector2f e = b - a, f = d - a, g = (a - b) + (c - d), h = p - a;

    auto cross2d = [](Vector2f a, Vector2f b) {
      return difference_of_products(a.x, b.y, a.y, b.x);
    };

    Float k2 = cross2d(g, f);
    Float k1 = cross2d(e, f) + cross2d(h, g);
    Float k0 = cross2d(h, e);

    if (std::abs(k2) < 0.001f) {
      if (std::abs(e.x * k1 - g.x * k2) < 1e-5f) {
        return {(h.y * k1 + f.y * k0) / (e.y * k1 - g.y * k0), -k0 / k1};
      } else {
        return {(h.x * k1 + f.x * k0) / (e.x * k1 - g.x * k0), -k0 / k1};
      }
    }

    Float v0 = NAN, v1 = NAN;
    if (!quadratic(k2, k1, k0, &v0, &v1)) {
      return {0, 0};
    }

    Float u = (h.x - f.x * v0) / (e.x + g.x * v0);
    if (u < 0 || u > 1 || v0 < 0 || v0 > 1) {
      return {(h.x - f.x * v1) / (e.x + g.x * v1), v1};
    }
    return {u, v0};
  }

  template <typename T>
  SPECULA_CPU_GPU inline auto distance(Point2<T> p1, Point2<T> p2) ->
      typename TupleLength<T>::type {
    return length(p1 - p2);
  }
  template <typename T>
  SPECULA_CPU_GPU inline auto distance_squared(Point2<T> p1, Point2<T> p2) ->
      typename TupleLength<T>::type {
    return length_squared(p1 - p2);
  }

} // namespace specula

template <typename T> struct fmt::formatter<specula::Point2<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::Point2<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "({}, {})", v.x, v.y);
  }
};

template <typename T> struct fmt::formatter<specula::Vector2<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::Vector2<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "({}, {})", v.x, v.y);
  }
};

#endif // SPECULA_UTIL_VECMATH_TUPLE2_HPP
