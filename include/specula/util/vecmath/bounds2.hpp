/**
 * @file bounds2.hpp
 * @brief Bounds2 class and related functions
 *
 * This file defines the Bounds2 class, which represents a 2D axis-aligned bounding box.
 * It includes various methods for manipulating and querying bounding boxes, as well as
 * utility functions for operations such as union, intersection, and containment checks.
 */

#ifndef INCLUDE_VECMATH_BOUNDS2_HPP_
#define INCLUDE_VECMATH_BOUNDS2_HPP_

#include "specula/util/math/functions.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {
  /**
   * @class Bounds2
   * @brief A class representing a 2D axis-aligned bounding box.
   *
   * A `Bounds2` object is defined by two points: \f$pMin\f$ and \f$pMax\f$, which represent
   * the minimum and maximum corners of the bounding box, respectively. The bounding box includes
   * all points \f$p\f$ such that \f$pMin \leq p \leq pMax\f$.
   *
   * The class provides various methods for manipulating and querying bounding boxes, such as
   * calculating the diagonal, area, checking for emptiness, and more.
   *
   * @tparam T The type of the coordinates (e.g., float, int).
   */
  template <typename T> class Bounds2 {
  public:
    /// @brief Default constructor initializes an empty bounding box.
    SPECULA_CPU_GPU Bounds2() {
      T min_num = std::numeric_limits<T>::lowest();
      T max_num = std::numeric_limits<T>::max();

      pmin = Point2<T>(max_num, max_num);
      pmax = Point2<T>(min_num, min_num);
    }

    /**
     * @brief Constructs a bounding box that encloses a single point.
     *
     * @param p The point to enclose.
     */
    SPECULA_CPU_GPU explicit Bounds2(Point2<T> p) : pmin(p), pmax(p) {}

    /**
     * @brief Constructs a bounding box that encloses two points.
     *
     * @param p1 The first point.
     * @param p2 The second point.
     */
    SPECULA_CPU_GPU Bounds2(Point2<T> p1, Point2<T> p2) {
      pmin = Point2<T>(min(p1, p2));
      pmax = Point2<T>(max(p1, p2));
    }

    /**
     * @brief Converts a bounding box of one type to another.
     *
     * @tparam U The type of the input bounding box.
     * @param b The bounding box to convert.
     */
    template <typename U> SPECULA_CPU_GPU explicit Bounds2(Bounds2<U> b) {
      if (b.is_empty()) {
        *this = Bounds2<T>();
      } else {
        pmin = Point2<T>(b.pmin);
        pmax = Point2<T>(b.pmax);
      }
    }

    /**
     * @brief Gets the diagonal vector of the bounding box.
     *
     * @return The diagonal vector from `pmin` to `pmax`.
     */
    SPECULA_CPU_GPU Vector2<T> diagonal() const { return pmax - pmin; }

    /**
     * @brief Computes the area of the bounding box.
     *
     * @return The area of the bounding box.
     */
    SPECULA_CPU_GPU T area() const {
      Vector2<T> d = diagonal();
      return d.x * d.y;
    }

    /**
     * @brief Checks if the bounding box is empty.
     *
     * @return True if the bounding box is empty, false otherwise.
     */
    SPECULA_CPU_GPU bool is_empty() const { return (pmin.x >= pmax.x) || (pmin.y >= pmax.y); }

    /**
     * @brief Checks if the bounding box is degenerate (invalid).
     *
     * A bounding box is considered degenerate if its minimum corner is greater than its maximum
     * corner in any dimension.
     *
     * @return True if the bounding box is degenerate, false otherwise.
     */
    SPECULA_CPU_GPU bool is_degenerate() const { return (pmin.x > pmax.x) || (pmin.y > pmax.y); }

    /**
     * @brief Gets the index of the dimension with the maximum extent.
     *
     * The function returns 0 for the x-dimension and 1 for the y-dimension.
     *
     * @return The index of the dimension with the maximum extent.
     */
    SPECULA_CPU_GPU int max_dimension() const {
      Vector2<T> d = diagonal();
      if (d.x > d.y) {
        return 0;
      } else {
        return 1;
      }
    }

    /**
     * @brief Get the corner of the bounding box specified by the index.
     *
     * The corners are indexed as follows:
     * - 0: \f$\left(pMin_x, pMin_y\right)\f$
     * - 1: \f$\left(pMax_x, pMin_y\right)\f$
     * - 2: \f$\left(pMin_x, pMax_y\right)\f$
     * - 3: \f$\left(pMax_x, pMax_y\right)\f$
     *
     * @param corner The index of the corner (0 to 3).
     * @return A `Point2<T>` representing the specified corner.
     */
    SPECULA_CPU_GPU Point2<T> corner(int corner) const {
      DASSERT(corner >= 0 && corner < 4);
      return Point2<T>((*this)[(corner & 1)].x, (*this)[(corner & 2) ? 1 : 0].y);
    }

    /**
     * @brief Linearly interpolates between the minimum and maximum corners of the bounding box.
     *
     * @param t The interpolation parameter, where (0,0) corresponds to `pmin` and (1,1) corresponds
     * to `pmax`.
     * @return The point resulting from the linear interpolation.
     */
    SPECULA_CPU_GPU Point2<T> lerp(Point2<T> t) const {
      return Point2<T>(specula::lerp(t.x, pmin.x, pmax.x), specula::lerp(t.y, pmin.y, pmax.y));
    }

    /**
     * @brief Computes the relative offset of a point within the bounding box.
     *
     * The offset is calculated as \f$\frac{p - pMin}{pMax - pMin}\f$ for each dimension. So the
     * offset is in the range \f$[0, 1]\f$ if the point is inside the bounding box. With \f$(0,
     * 0)\f$ corresponding to `pmin` and \f$(1, 1)\f$ corresponding to `pmax`.
     *
     * @param p The point for which to compute the offset.
     * @return A `Vector2<T>` representing the relative offset of the point within the bounding box.
     */
    SPECULA_CPU_GPU Vector2<T> offset(Point2<T> p) const {
      Vector2<T> o = p - pmin;
      if (pmax.x > pmin.x) {
        o.x /= pmax.x - pmin.x;
      }
      if (pmax.y > pmin.y) {
        o.y /= pmax.y - pmin.y;
      }
      return o;
    }

    /**
     * @brief Computes the bounding sphere of the bounding box.
     *
     * @param[out] c The center of the bounding sphere.
     * @param[out] rad The radius of the bounding sphere.
     */
    SPECULA_CPU_GPU void bounding_sphere(Point2<T> *c, Float *rad) const {
      DASSERT(c != nullptr && rad != nullptr);
      *c = (pmin + pmax) / 2;
      *rad = distance(*c, pmax);
    }

    /**
     * @brief Access operator to get the minimum or maximum corner of the bounding box.
     *
     * @param i The index (0 for `pmin`, 1 for `pmax`).
     * @return The corresponding corner point.
     */
    SPECULA_CPU_GPU Point2<T> operator[](int i) const {
      DASSERT(i == 0 || i == 1);
      return (i == 0) ? pmin : pmax;
    }

    /**
     * @brief Access operator to get a reference to the minimum or maximum corner of the bounding
     * box.
     *
     * @param i The index (0 for `pmin`, 1 for `pmax`).
     * @return A reference to the corresponding corner point.
     */
    SPECULA_CPU_GPU Point2<T> &operator[](int i) {
      DASSERT(i == 0 || i == 1);
      return (i == 0) ? pmin : pmax;
    }

    /**
     * @brief Equality operator to compare two bounding boxes.
     *
     * @param b The bounding box to compare with.
     * @return True if the bounding boxes are equal, false otherwise.
     */
    SPECULA_CPU_GPU bool operator==(const Bounds2<T> &b) const {
      return b.pmin == pmin && b.pmax == pmax;
    }

    /**
     * @brief Inequality operator to compare two bounding boxes.
     *
     * @param b The bounding box to compare with.
     * @return True if the bounding boxes are not equal, false otherwise.
     */
    SPECULA_CPU_GPU bool operator!=(const Bounds2<T> &b) const {
      return b.pmin != pmin || b.pmax != pmax;
    }

    Point2<T> pmin, ///< The minimum corner of the bounding box.
        pmax;       ///< The maximum corner of the bounding box.
  };

  /// @brief Type alias for a bounding box with `float` coordinates.
  typedef Bounds2<Float> Bounds2f;
  /// @brief Type alias for a bounding box with `int` coordinates.
  typedef Bounds2<int> Bounds2i;

  /**
   * @brief Computes the union of two bounding boxes.
   *
   * @tparam T The type of the bounding box coordinates.
   * @param b1 The first bounding box.
   * @param b2 The second bounding box.
   * @return A new bounding box that encompasses both input bounding boxes.
   */
  template <typename T>
  SPECULA_CPU_GPU inline Bounds2<T> bounds_union(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    Bounds2<T> ret;
    ret.pmin = min(b1.pmin, b2.pmin);
    ret.pmax = max(b1.pmax, b2.pmax);
    return ret;
  }

  /**
   * @brief Computes the union of a bounding box and a point.
   *
   * @tparam T The type of the bounding box coordinates.
   * @param b The bounding box.
   * @param p The point to include in the bounding box.
   * @return A new bounding box that encompasses both the input bounding box and the point.
   */
  template <typename T>
  SPECULA_CPU_GPU inline Bounds2<T> bounds_union(const Bounds2<T> &b, const Point2<T> &p) {
    Bounds2<T> ret;
    ret.pmin = min(b.pmin, p);
    ret.pmax = max(b.pmax, p);
    return ret;
  }

  /**
   * @brief Computes the intersection of two bounding boxes.
   *
   * @tparam T The type of the bounding box coordinates.
   * @param b1 The first bounding box.
   * @param b2 The second bounding box.
   * @return A new bounding box that represents the overlapping area of the two input bounding
   * boxes.
   */
  template <typename T>
  SPECULA_CPU_GPU inline Bounds2<T> intersect(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    Bounds2<T> ret;
    ret.pmin = max(b1.pmin, b2.pmin);
    ret.pmax = min(b1.pmax, b2.pmax);
    return ret;
  }

  /**
   * @brief Checks if two bounding boxes overlap.
   *
   * @tparam T The type of the bounding box coordinates.
   * @param b1 The first bounding box.
   * @param b2 The second bounding box.
   * @return True if the bounding boxes overlap, false otherwise.
   */
  template <typename T>
  SPECULA_CPU_GPU inline bool overlaps(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    bool x = (b1.pmax.x >= b2.pmin.x) && (b1.pmin.x <= b2.pmax.x);
    bool y = (b1.pmax.y >= b2.pmin.y) && (b1.pmin.y <= b2.pmax.y);
    return (x && y);
  }

  /**
   * @brief Checks if a point is inside a bounding box.
   *
   * @tparam T The type of the bounding box coordinates.
   * @param pt The point to check.
   * @param b The bounding box.
   * @return True if the point is inside the bounding box, false otherwise.
   */
  template <typename T>
  SPECULA_CPU_GPU inline bool inside(const Point2<T> pt, const Bounds2<T> &b) {
    return (pt.x >= b.pmin.x && pt.x <= b.pmax.x && pt.y >= b.pmin.y && pt.y <= b.pmax.y);
  }

  /**
   * @brief Checks if one bounding box is inside another bounding box.
   *
   * @tparam T The type of the bounding box coordinates.
   * @param b1 The bounding box to check if it is inside `b2`.
   * @param b2 The bounding box to check against.
   * @return True if `b1` is inside `b2`, false otherwise.
   */
  template <typename T>
  SPECULA_CPU_GPU inline bool inside(const Bounds2<T> &b1, const Bounds2<T> &b2) {
    return (b1.pmin.x >= b2.pmin.x && b1.pmax.x <= b2.pmax.x && b1.pmin.y >= b2.pmin.y &&
            b1.pmax.y <= b2.pmax.y);
  }

  /**
   * @brief Checks if a point is inside a bounding box, excluding the maximum boundary.
   *
   * @tparam T The type of the bounding box coordinates.
   * @param pt The point to check.
   * @param b The bounding box.
   * @return True if the point is inside the bounding box (excluding the maximum boundary), false
   */
  template <typename T>
  SPECULA_CPU_GPU inline bool inside_exclusive(Point2<T> pt, const Bounds2<T> &b) {
    return (pt.x >= b.pmin.x && pt.x < b.pmax.x && pt.y >= b.pmin.y && pt.y < b.pmax.y);
  }

  /**
   * @brief Expands a bounding box by a given delta in all directions.
   *
   * @tparam T The type of the bounding box coordinates.
   * @tparam U The type of the delta value.
   * @param b The bounding box to expand.
   * @param delta The amount to expand the bounding box in all directions.
   * @return A new bounding box that is expanded by the given delta in all directions.
   */
  template <typename T, typename U>
  SPECULA_CPU_GPU inline Bounds2<T> expand(const Bounds2<T> &b, U delta) {
    Bounds2<T> ret;

    ret.pmin = b.pmin - Vector2<T>(delta, delta);
    ret.pmax = b.pmax + Vector2<T>(delta, delta);

    return ret;
  }
} // namespace specula

template <typename T> struct fmt::formatter<specula::Bounds2<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Bounds2<T> &b, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ {} - {} ]", b.pmin, b.pmax);
  }
};

#endif // INCLUDE_VECMATH_BOUNDS2_HPP_
