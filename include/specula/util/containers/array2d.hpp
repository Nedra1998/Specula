#ifndef INCLUDE_CONTAINERS_ARRAY2D_HPP_
#define INCLUDE_CONTAINERS_ARRAY2D_HPP_

#include <algorithm>
#include <type_traits>

#include "specula.hpp"
#include "util/check.hpp"
#include "util/pstd/pmr.hpp"
#include "util/pstd/vector.hpp"
#include "util/vecmath/bounds2.hpp"

namespace specula {
  template <typename T> class Array2D {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using allocator_type = pstd::pmr::polymorphic_allocator<std::byte>;

    Array2D(allocator_type allocator = {}) : Array2D({{0, 0}, {0, 0}}, allocator) {}

    Array2D(Bounds2i extent, allocator_type allocator = {}) : extent(extent), allocator(allocator) {
      int n = extent.area();
      values = allocator.allocate_object<T>(n);
      for (int i = 0; i < n; ++i) {
        allocator.construct(values + i);
      }
    }

    Array2D(Bounds2i extent, T def, allocator_type allocator = {}) : Array2D(extent, allocator) {
      std::fill(begin(), end(), def);
    }

    template <typename InputIt,
              typename = typename std::enable_if_t<
                  !std::is_integral_v<InputIt> &&
                  std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<
                                                               InputIt>::iterator_category>::value>>
    Array2D(InputIt first, InputIt last, int nx, int ny, allocator_type allocator = {})
        : Array2D({{0, 0}, {nx, ny}}, allocator) {
      std::copy(first, last, begin());
    }

    Array2D(int nx, int ny, allocator_type allocator = {})
        : Array2D({{0, 0}, {nx, ny}}, allocator) {}
    Array2D(int nx, int ny, T def, allocator_type allocator = {})
        : Array2D({{0, 0}, {nx, ny}}, def, allocator) {}
    Array2D(const Array2D &other, allocator_type allocator = {})
        : Array2D(other.begin(), other.end(), other.xsize(), other.ysize(), allocator) {}

    Array2D(Array2D &&other, allocator_type allocator = {})
        : extent(other.extent), allocator(allocator) {
      if (allocator == other.allocator) {
        values = other.values;
        other.extent = Bounds2i({0, 0}, {0, 0});
        other.values = nullptr;
      } else {
        values = allocator.allocate_object<T>(extent.area());
        std::copy(other.begin(), other.end(), begin());
      }
    }

    ~Array2D() {
      int n = extent.area();
      for (int i = 0; i < n; ++i) {
        allocator.destroy(values + i);
      }
      allocator.deallocate_object(values, n);
    }

    SPECULA_CPU_GPU int size() const { return extent.area(); }
    SPECULA_CPU_GPU int xsize() const { return extent.pmax.x - extent.pmin.x; }
    SPECULA_CPU_GPU int ysize() const { return extent.pmax.y - extent.pmin.y; }

    SPECULA_CPU_GPU iterator begin() { return values; }
    SPECULA_CPU_GPU iterator end() { return begin() + size(); }
    SPECULA_CPU_GPU const_iterator begin() const { return values; }
    SPECULA_CPU_GPU const_iterator end() const { return begin() + size(); }
    SPECULA_CPU_GPU const_iterator cbegin() const { return values; }
    SPECULA_CPU_GPU const_iterator cend() const { return begin() + size(); }

    SPECULA_CPU_GPU operator pstd::span<T>() { return pstd::span<T>(values, size()); }
    SPECULA_CPU_GPU operator pstd::span<const T>() const {
      return pstd::span<const T>(values, size());
    }

    Array2D &operator=(const Array2D &other) = delete;

    Array2D &operator=(Array2D &&other) {
      if (allocator == other.allocator) {
        pstd::swap(extent, other.extent);
        pstd::swap(values, other.values);
      } else if (extent == other.extent) {
        int n = extent.area();
        for (int i = 0; i < n; ++i) {
          allocator.destroy(values + i);
          allocator.construct(values + i, other.values[i]);
        }
      } else {
        int n = extent.area();
        for (int i = 0; i < n; ++i) {
          allocator.destroy(values + i);
        }
        allocator.deallocate_object(values, n);

        int no = other.extent.area();
        values = allocator.allocate_object<T>(no);
        for (int i = 0; i < no; ++i) {
          allocator.construct(values + i, other.values[i]);
        }
        extent = other.extent;
      }

      return *this;
    }

    SPECULA_CPU_GPU T &operator[](Point2i p) {
      DASSERT(inside_exclusive(p, extent));
      p.x -= extent.pmin.x;
      p.y -= extent.pmin.y;
      return values[p.x + (extent.pmax.x - extent.pmin.x) * p.y];
    }
    SPECULA_CPU_GPU const T &operator[](Point2i p) const {
      DASSERT(inside_exclusive(p, extent));
      p.x -= extent.pmin.x;
      p.y -= extent.pmin.y;
      return values[p.x + (extent.pmax.x - extent.pmin.x) * p.y];
    }

    SPECULA_CPU_GPU T &operator()(int x, int y) { return (*this)[{x, y}]; }
    SPECULA_CPU_GPU const T &operator()(int x, int y) const { return (*this)[{x, y}]; }

  private:
    Bounds2i extent;
    Allocator allocator;
    T *values;
  };
} // namespace specula

#endif // INCLUDE_CONTAINERS_ARRAY2D_HPP_
