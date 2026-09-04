#ifndef SPECULA_UTIL_VECMATH_BOUNDS2_HPP
#define SPECULA_UTIL_VECMATH_BOUNDS2_HPP

#include <iterator>
#include <limits>

#include "specula/macros.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {
  template <typename T> class Bounds2 {
  public:
    SPECULA_CPU_GPU Bounds2() {
      T min_num = std::numeric_limits<T>::lowest();
      T max_num = std::numeric_limits<T>::max();
      p_min = Point2<T>(max_num, max_num);
      p_max = Point2<T>(min_num, min_num);
    }

    SPECULA_CPU_GPU explicit Bounds2(Point2<T> p) : p_min(p), p_max(p) {}
    SPECULA_CPU_GPU Bounds2(Point2<T> p1, Point2<T> p2) : p_min(min(p1, p2)), p_max(max(p1, p2)) {}
    template <typename U> SPECULA_CPU_GPU explicit Bounds2(const Bounds2<U> &b) {
      if (b.is_empty()) {
        *this = Bounds2<T>();
      } else {
        p_min = Point2<T>(b.p_min);
        p_max = Point2<T>(b.p_max);
      }
    }

    SPECULA_CPU_GPU [[nodiscard]] Vector2<T> diagonal() const { return p_max - p_min; }
    SPECULA_CPU_GPU [[nodiscard]] T area() const {
      Vector2<T> d = p_max - p_min;
      return d.x * d.y;
    }

    SPECULA_CPU_GPU [[nodiscard]] bool is_empty() const {
      return p_min.x >= p_max.x || p_min.y >= p_max.y;
    }
    SPECULA_CPU_GPU [[nodiscard]] bool is_degenerate() const {
      return p_min.x > p_max.x || p_min.y > p_max.y;
    }

    SPECULA_CPU_GPU [[nodiscard]] int max_dimension() const {
      Vector2<T> diag = diagonal();
      return diag.x > diag.y ? 0 : 1;
    }

    SPECULA_CPU_GPU Point2<T> &operator[](int i) {
      DASSERT(i == 0 || i == 1);
      return (i == 0) ? p_min : p_max;
    }
    SPECULA_CPU_GPU Point2<T> operator[](int i) const {
      DASSERT(i == 0 || i == 1);
      return (i == 0) ? p_min : p_max;
    }

    SPECULA_CPU_GPU bool operator==(const Bounds2<T> &b) const {
      return b.p_min == p_min && b.p_max == p_max;
    }
    SPECULA_CPU_GPU bool operator!=(const Bounds2<T> &b) const {
      return b.p_min != p_min || b.p_max != p_max;
    }

    SPECULA_CPU_GPU [[nodiscard]] Point2<T> corner(int corner) const {
      DASSERT(corner >= 0 && corner < 4);
      return Point2<T>((*this)[(corner & 1)].x, (*this)[((corner & 2) != 0) ? 1 : 0].y);
    }

    SPECULA_CPU_GPU [[nodiscard]] Point2<T> lerp(Point2f t) const {
      return Point2<T>(lerp(t.x, p_min.x, p_max.x), lerp(t.y, p_min.y, p_max.y));
    }

    SPECULA_CPU_GPU [[nodiscard]] Vector2<T> offset(Point2<T> p) const {
      Vector2<T> o = p - p_min;
      if (p_max.x > p_min.x) {
        o.x /= p_max.x - p_min.x;
      }
      if (p_max.y > p_min.y) {
        o.y /= p_max.y - p_min.y;
      }
      return o;
    }

    SPECULA_CPU_GPU void bounding_sphere(Point2<T> *c, Float *rad) const {
      *c = (p_min + p_max) / 2;
      *rad = inside(*c, *this) ? distance(*c, p_max) : 0;
    }

    Point2<T> p_min, p_max;
  };

  using Bounds2f = Bounds2<Float>;
  using Bounds2i = Bounds2<int>;

  class Bounds2iIterator : public std::forward_iterator_tag {
  public:
    SPECULA_CPU_GPU Bounds2iIterator(const Bounds2i &b, const Point2i &pt) : p(pt), bounds(&b) {}
    SPECULA_CPU_GPU Bounds2iIterator operator++() {
      advance();
      return *this;
    }
    SPECULA_CPU_GPU Bounds2iIterator operator++(int) {
      Bounds2iIterator old = *this;
      advance();
      return old;
    }

    SPECULA_CPU_GPU bool operator==(const Bounds2iIterator &bi) const {
      return p == bi.p && bounds == bi.bounds;
    }

    SPECULA_CPU_GPU bool operator!=(const Bounds2iIterator &bi) const {
      return p != bi.p || bounds != bi.bounds;
    }

    SPECULA_CPU_GPU Point2i operator*() const { return p; }

  private:
    SPECULA_CPU_GPU void advance() {
      ++p.x;
      if (p.x == bounds->p_max.x) {
        p.x = bounds->p_min.x;
        ++p.y;
      }
    }

    Point2i p;
    const Bounds2i *bounds;
  };

  template <typename T>
  SPECULA_CPU_GPU inline Bounds2<T> bunion(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    Bounds2<T> ret;
    ret.p_min = min(b1.p_min, b2.p_min);
    ret.p_max = max(b1.p_max, b2.p_max);
    return ret;
  }

  template <typename T>
  SPECULA_CPU_GPU inline Bounds2<T> bunion(const Bounds2<T> &b, const Point2<T> &p) {
    Bounds2<T> ret;
    ret.p_min = min(b.p_min, p);
    ret.p_max = max(b.p_max, p);
    return ret;
  }

  template <typename T>
  SPECULA_CPU_GPU inline Bounds2<T> intersect(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    Bounds2<T> ret;
    ret.p_min = max(b1.p_min, b2.p_min);
    ret.p_max = min(b1.p_max, b2.p_max);
    return ret;
  }

  template <typename T>
  SPECULA_CPU_GPU inline bool overlaps(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    bool x = (b1.p_max.x >= b2.p_min.x) && (b1.p_min.x <= b2.p_max.x);
    bool y = (b1.p_max.y >= b2.p_min.y) && (b1.p_min.y <= b2.p_max.y);
    return (x && y);
  }

  template <typename T> SPECULA_CPU_GPU inline bool inside(Point2<T> pt, const Bounds2<T> &b) {
    return (pt.x >= b.p_min.x && pt.x <= b.p_max.x && pt.y >= b.p_min.y && pt.y <= b.p_max.y);
  }

  template <typename T> SPECULA_CPU_GPU inline bool inside(Bounds2<T> b1, const Bounds2<T> &b2) {
    return (b1.p_min.x >= b2.p_min.x && b1.p_max.x <= b2.p_max.x && b1.p_min.y >= b2.p_min.y &&
            b1.p_max.y <= b2.p_max.y);
  }

  template <typename T>
  SPECULA_CPU_GPU inline bool inside_exclusive(Point2<T> pt, const Bounds2<T> &b) {
    return (pt.x >= b.p_min.x && pt.x < b.p_max.x && pt.y >= b.p_min.y && pt.y < b.p_max.y);
  }

  template <typename T, typename U>
  SPECULA_CPU_GPU inline Bounds2<T> expand(const Bounds2<T> &b, U delta) {
    Bounds2<T> ret;
    ret.p_min = b.p_min - Vector2<T>(delta, delta);
    ret.p_max = b.p_max + Vector2<T>(delta, delta);
    return ret;
  }

  SPECULA_CPU_GPU inline Bounds2iIterator begin(const Bounds2i &b) { return {b, b.p_min}; }
  SPECULA_CPU_GPU inline Bounds2iIterator end(const Bounds2i &b) {
    Point2i p_end(b.p_min.x, b.p_max.y);
    if (b.p_min.x >= b.p_max.x || b.p_min.y >= b.p_max.y) {
      p_end = b.p_min;
    }
    return {b, p_end};
  }

} // namespace specula

template <typename T> struct fmt::formatter<specula::Bounds2<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::Bounds2<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ {} - {} ]", v.p_min, v.p_max);
  }
};

#endif // SPECULA_UTIL_VECMATH_BOUNDS2_HPP
