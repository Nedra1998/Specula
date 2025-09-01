/**
 * @file bounds2i_iterator.hpp
 * @brief Iterator class for Bounds2i
 *
 * This file defines the `Bounds2iIterator` class, which provides an iterator
 * for iterating over all integer points within a 2D axis-aligned bounding box.
 */

#ifndef INCLUDE_VECMATH_BOUNDS2I_ITERATOR_HPP_
#define INCLUDE_VECMATH_BOUNDS2I_ITERATOR_HPP_

#include <iterator>

#include "specula/specula.hpp"
#include "specula/util/vecmath/bounds2.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {
  /**
   * @class Bounds2iIterator
   * @brief A forward iterator for iterating over all integer points within a 2D bounding box.
   *
   * This iterator allows traversal of all integer points contained within a `Bounds2i` object,
   * which represents a 2D axis-aligned bounding box defined by two integer points: `pmin` and
   * `pmax`.
   *
   * The iterator starts at the minimum corner (`pmin`) and iterates through all points
   * up to, but not including, the maximum corner (`pmax`), moving in a row-major order.
   *
   */
  class Bounds2iIterator : public std::forward_iterator_tag {
  public:
    /**
     * @brief Constructs an iterator for the given bounding box starting at the specified point.
     *
     * @param b The bounding box to iterate over.
     * @param pt The starting point for the iterator.
     */
    SPECULA_CPU_GPU Bounds2iIterator(const Bounds2i &b, const Point2i &pt) : p(pt), bounds(&b) {}

    /// @brief Advances the iterator to the next point in the bounding box.
    SPECULA_CPU_GPU Bounds2iIterator operator++() {
      advance();
      return *this;
    }

    /// @brief Advances the iterator to the next point in the bounding box (postfix).
    SPECULA_CPU_GPU Bounds2iIterator operator++(int) {
      Bounds2iIterator old = *this;
      advance();
      return old;
    }

    /**
     * @brief Equality comparison operator.
     *
     * @param bi The other iterator to compare with.
     * @return True if both iterators point to the same position in the same bounding box, false
     * otherwise.
     */
    SPECULA_CPU_GPU bool operator==(const Bounds2iIterator &bi) const {
      return p == bi.p && bounds == bi.bounds;
    }

    /**
     * @brief Inequality comparison operator.
     *
     * @param bi The other iterator to compare with.
     * @return True if the iterators point to different positions or different bounding boxes, false
     * otherwise.
     */
    SPECULA_CPU_GPU bool operator!=(const Bounds2iIterator &bi) const {
      return p != bi.p || bounds != bi.bounds;
    }

    /// @brief Dereference operator to access the current point.
    SPECULA_CPU_GPU Point2i operator*() const { return p; }

  private:
    SPECULA_CPU_GPU void advance() {
      ++p.x;
      if (p.x == bounds->pmax.x) {
        p.x = bounds->pmin.x;
        ++p.y;
      }
    }

    Point2i p;
    const Bounds2i *bounds;
  };

  /**
   * @brief Returns an iterator to the beginning of the bounding box.
   *
   * @param b The bounding box to iterate over.
   * @return A `Bounds2iIterator` pointing to the first point in the bounding box.
   */
  SPECULA_CPU_GPU inline Bounds2iIterator begin(const Bounds2i &b) {
    return Bounds2iIterator(b, b.pmin);
  }

  /**
   * @brief Returns an iterator to the end of the bounding box.
   *
   * @param b The bounding box to iterate over.
   * @return A `Bounds2iIterator` pointing just past the last point in the bounding box.
   */
  SPECULA_CPU_GPU inline Bounds2iIterator end(const Bounds2i &b) {
    Point2i pend(b.pmin.x, b.pmax.y);
    if (b.pmin.x >= b.pmax.x || b.pmin.y >= b.pmax.y) {
      pend = b.pmin;
    }
    return Bounds2iIterator(b, pend);
  }
} // namespace specula

#endif // INCLUDE_VECMATH_BOUNDS2I_ITERATOR_HPP_
