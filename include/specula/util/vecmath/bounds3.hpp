#ifndef SPECULA_UTIL_VECMATH_BOUNDS3_HPP
#define SPECULA_UTIL_VECMATH_BOUNDS3_HPP

#include <algorithm>
#include <cmath>
#include <limits>

#include "specula/macros.hpp"
#include "specula/util/vecmath/tuple3.hpp"

namespace specula {
  template <typename T> class Bounds3 {
  public:
    SPECULA_CPU_GPU Bounds3() {
      T min_num = std::numeric_limits<T>::lowest();
      T max_num = std::numeric_limits<T>::max();
      p_min = Point3<T>(max_num, max_num, max_num);
      p_max = Point3<T>(min_num, min_num, min_num);
    }

    SPECULA_CPU_GPU explicit Bounds3(Point3<T> p) : p_min(p), p_max(p) {}
    SPECULA_CPU_GPU Bounds3(Point3<T> p1, Point3<T> p2) : p_min(min(p1, p2)), p_max(max(p1, p2)) {}
    template <typename U> SPECULA_CPU_GPU explicit Bounds3(const Bounds3<U> &b) {
      if (b.is_empty()) {
        *this = Bounds3<T>();
      } else {
        p_min = Point3<T>(b.p_min);
        p_max = Point3<T>(b.p_max);
      }
    }

    SPECULA_CPU_GPU [[nodiscard]] Vector3<T> diagonal() const { return p_max - p_min; }
    SPECULA_CPU_GPU [[nodiscard]] T surface_area() const {
      Vector3<T> d = diagonal();
      return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }
    SPECULA_CPU_GPU [[nodiscard]] T volume() const {
      Vector3<T> d = diagonal();
      return d.x * d.y * d.z;
    }

    SPECULA_CPU_GPU [[nodiscard]] bool is_empty() const {
      return p_min.x >= p_max.x || p_min.y >= p_max.y || p_min.z >= p_max.z;
    }
    SPECULA_CPU_GPU [[nodiscard]] bool is_degenerate() const {
      return p_min.x > p_max.x || p_min.y > p_max.y || p_min.z > p_max.z;
    }

    SPECULA_CPU_GPU [[nodiscard]] int max_dimension() const {
      Vector3<T> d = diagonal();
      if (d.x > d.y && d.x > d.z) {
        return 0;
      } else if (d.y > d.z) {
        return 1;
      } else {
        return 2;
      }
    }

    SPECULA_CPU_GPU Point3<T> &operator[](int i) {
      DASSERT(i == 0 || i == 1);
      return (i == 0) ? p_min : p_max;
    }
    SPECULA_CPU_GPU Point3<T> operator[](int i) const {
      DASSERT(i == 0 || i == 1);
      return (i == 0) ? p_min : p_max;
    }

    SPECULA_CPU_GPU bool operator==(const Bounds3<T> &b) const {
      return b.p_min == p_min && b.p_max == p_max;
    }
    SPECULA_CPU_GPU bool operator!=(const Bounds3<T> &b) const {
      return b.p_min != p_min || b.p_max != p_max;
    }

    SPECULA_CPU_GPU [[nodiscard]] Point3<T> corner(int corner) const {
      DASSERT(corner >= 0 && corner < 8);
      return Point2<T>((*this)[(corner & 1)].x, (*this)[((corner & 2) != 0) ? 1 : 0].y,
                       (*this)[((corner & 4) != 0) ? 1 : 0].z);
    }

    SPECULA_CPU_GPU [[nodiscard]] Point3<T> lerp(Point3f t) const {
      return Point3<T>(lerp(t.x, p_min.x, p_max.x), lerp(t.y, p_min.y, p_max.y),
                       lerp(t.z, p_min.z, p_max.z));
    }

    SPECULA_CPU_GPU [[nodiscard]] Vector3<T> offset(Point3<T> p) const {
      Vector3<T> o = p - p_min;
      if (p_max.x > p_min.x) {
        o.x /= p_max.x - p_min.x;
      }
      if (p_max.y > p_min.y) {
        o.y /= p_max.y - p_min.y;
      }
      if (p_max.z > p_min.z) {
        o.z /= p_max.z - p_min.z;
      }
      return o;
    }

    SPECULA_CPU_GPU void bounding_sphere(Point3<T> *c, Float *rad) const {
      *c = (p_min + p_max) / 2;
      *rad = inside(*c, *this) ? distance(*c, p_max) : 0;
    }

    SPECULA_CPU_GPU bool intersect_p(Point3f o, Vector3f d, Float t_max = INFINITY,
                                     Float *hitt0 = nullptr, Float *hitt1 = nullptr) const {
      Float t0 = 0, t1 = t_max;
      for (int i = 0; i < 3; ++i) {
        Float inv_ray_dir = 1 / d[i];
        Float t_near = (p_min[i] - o[i]) * inv_ray_dir;
        Float t_far = (p_max[i] - o[i]) * inv_ray_dir;

        if (t_near > t_far) {
          pstd::swap(t_near, t_far);
        }
        t_far *= 1 + 2 * gamma(3);

        t0 = t_near > t0 ? t_near : t0;
        t1 = t_far < t1 ? t_far : t1;
        if (t0 > t1) {
          return false;
        }
      }
      if (hitt0 != nullptr) {
        *hitt0 = t0;
      }
      if (hitt1 != nullptr) {
        *hitt1 = t1;
      }
      return true;
    }

    SPECULA_CPU_GPU bool intersect_p(Point3f o, Vector3f d, Float t_max, Vector3f inv_dir,
                                     const int dir_is_neg[3]) const {
      const Bounds3<Float> &bounds = *this;
      Float tx_min = (bounds[dir_is_neg[0]].x - o.x) * inv_dir.x;
      Float tx_max = (bounds[1 - dir_is_neg[0]].x - o.x) * inv_dir.x;
      Float ty_min = (bounds[dir_is_neg[1]].y - o.y) * inv_dir.y;
      Float ty_max = (bounds[1 - dir_is_neg[1]].y - o.y) * inv_dir.y;

      tx_max *= 1 + 2 * gamma(3);
      ty_max *= 1 + 2 * gamma(3);

      if (tx_min > ty_max || ty_min > tx_max) {
        return false;
      }
      tx_min = std::max(ty_min, tx_min);
      tx_max = std::min(ty_max, tx_max);

      Float tz_min = (bounds[dir_is_neg[2]].z - o.z) * inv_dir.z;
      Float tz_max = (bounds[1 - dir_is_neg[2]].z - o.z) * inv_dir.z;
      tz_max *= 1 + 2 * gamma(3);

      if (tx_min > tz_max || tz_min > tx_max) {
        return false;
      }
      tx_min = std::max(tz_min, tx_min);
      tx_max = std::min(tz_max, tx_max);
      return (tx_max < t_max) && (t_max > 0);
    }

    Point3<T> p_min, p_max;
  };

  using Bounds3f = Bounds3<Float>;
  using Bounds3i = Bounds3<int>;

  template <typename T>
  SPECULA_CPU_GPU inline Bounds3<T> bunion(const Bounds3<T> &b1, const Bounds3<T> &b2) {
    Bounds3<T> ret;
    ret.p_min = min(b1.p_min, b2.p_min);
    ret.p_max = max(b1.p_max, b2.p_max);
    return ret;
  }

  template <typename T>
  SPECULA_CPU_GPU inline Bounds3<T> bunion(const Bounds3<T> &b, const Point3<T> &p) {
    Bounds3<T> ret;
    ret.p_min = min(b.p_min, p);
    ret.p_max = max(b.p_max, p);
    return ret;
  }

  template <typename T>
  SPECULA_CPU_GPU inline Bounds3<T> intersect(const Bounds3<T> &b1, const Bounds3<T> &b2) {
    Bounds3<T> ret;
    ret.p_min = max(b1.p_min, b2.p_min);
    ret.p_max = min(b1.p_max, b2.p_max);
    return ret;
  }

  template <typename T>
  SPECULA_CPU_GPU inline bool overlaps(const Bounds3<T> &b1, const Bounds3<T> &b2) {
    bool x = (b1.p_max.x >= b2.p_min.x) && (b1.p_min.x <= b2.p_max.x);
    bool y = (b1.p_max.y >= b2.p_min.y) && (b1.p_min.y <= b2.p_max.y);
    bool z = (b1.p_max.z >= b2.p_min.z) && (b1.p_min.z <= b2.p_max.z);
    return (x && y && z);
  }

  template <typename T> SPECULA_CPU_GPU inline bool inside(Point3<T> pt, const Bounds3<T> &b) {
    return (pt.x >= b.p_min.x && pt.x <= b.p_max.x && pt.y >= b.p_min.y && pt.y <= b.p_max.y &&
            pt.z >= b.p_min.z && pt.z <= b.p_max.z);
  }

  template <typename T> SPECULA_CPU_GPU inline bool inside(Bounds3<T> b1, const Bounds3<T> &b2) {
    return (b1.p_min.x >= b2.p_min.x && b1.p_max.x <= b2.p_max.x && b1.p_min.y >= b2.p_min.y &&
            b1.p_max.y <= b2.p_max.y && b1.p_min.z >= b2.p_min.z && b1.p_max.z <= b2.p_max.z);
  }

  template <typename T>
  SPECULA_CPU_GPU inline bool inside_exclusive(Point3<T> pt, const Bounds3<T> &b) {
    return (pt.x >= b.p_min.x && pt.x < b.p_max.x && pt.y >= b.p_min.y && pt.y < b.p_max.y &&
            pt.z >= b.p_min.z && pt.z < b.p_max.z);
  }

  template <typename T, typename U>
  SPECULA_CPU_GPU inline Bounds3<T> expand(const Bounds3<T> &b, U delta) {
    Bounds3<T> ret;
    ret.p_min = b.p_min - Vector3<T>(delta, delta, delta);
    ret.p_max = b.p_max + Vector3<T>(delta, delta, delta);
    return ret;
  }

  template <typename T, typename U>
  SPECULA_CPU_GPU inline auto distance_squared(Point3<T> p, const Bounds3<U> &b) {
    using TDist = decltype(T{} - U{});
    TDist dx = std::max<TDist>({0, b.p_min.x - p.x, p.x - b.p_max.x});
    TDist dy = std::max<TDist>({0, b.p_min.y - p.y, p.y - b.p_max.y});
    TDist dz = std::max<TDist>({0, b.p_min.z - p.z, p.z - b.p_max.z});
    return sqr(dx) + sqr(dy) + sqr(dz);
  }

  template <typename T, typename U>
  SPECULA_CPU_GPU inline auto distance(Point3<T> p, const Bounds3<U> &b) {
    auto dist = distance_squared(p, b);
    using TDist = typename TupleLength<decltype(dist)>::type;
    return std::sqrt(TDist(dist));
  }

} // namespace specula

template <typename T> struct fmt::formatter<specula::Bounds3<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::Bounds3<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ {} - {} ]", v.p_min, v.p_max);
  }
};

#endif // SPECULA_UTIL_VECMATH_BOUNDS3_HPP
