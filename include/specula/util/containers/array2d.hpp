#ifndef SPECULA_UTIL_CONTAINERS_ARRAY2D_HPP
#define SPECULA_UTIL_CONTAINERS_ARRAY2D_HPP

#include <iterator>
#include <type_traits>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  template <typename T> class Array2D {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using allocator_type = pstd::pmr::polymorphic_allocator<std::byte>;

    Array2D(allocator_type allocator = {}) : Array2D({{0, 0}, {0, 0}}, allocator) {}
    Array2D(Bounds2i extent, Allocator allocator = {}) : extent(extent), allocator(allocator) {
      int n = extent.area();
      values = allocator.allocate_object<T>(n);
      for (int i = 0; i < n; ++i) {
        allocator.construct(values + i);
      }
    }

    Array2D(Bounds2i extent, T def, allocator_type allocator = {}) : Array2D(extent, allocator) {
      std::fill(begin(), end(), def);
    }

    template <typename InputIt>
    Array2D(InputIt first, InputIt last, int nx, int ny, allocator_type allocator = {})
      requires(!std::is_integral_v<InputIt> &&
               std::is_base_of_v<std::input_iterator_tag,
                                 typename std::iterator_traits<InputIt>::iterator_category>)
        : Array2D({{0, 0}, {nx, ny}}, allocator) {
      std::copy(first, last, begin());
    }

    Array2D(int nx, int ny, allocator_type allocator = {})
        : Array2D({{0, 0}, {nx, ny}}, allocator) {}
    Array2D(int nx, int ny, T def, allocator_type allocator = {})
        : Array2D({{0, 0}, {nx, ny}}, def, allocator) {}
    Array2D(const Array2D &a, allocator_type allocator = {})
        : Array2D(a.begin(), a.end(), a.xsize(), a.ysize(), allocator) {}

    Array2D(Array2D &&a, allocator_type allocator = {}) noexcept
        : extent(a.extent), allocator(allocator) {
      if (allocator == a.allocator) {
        values = a.values;
        a.extent = Bounds2i({0, 0}, {0, 0});
        a.values = nullptr;
      } else {
        values = allocator.allocate_object<T>(extent.area());
        std::copy(a.begin(), a.end(), begin());
      }
    }

    ~Array2D() {
      int n = extent.area();
      for (int i = 0; i < n; ++i) {
        allocator.destroy(values + i);
      }
      allocator.deallocate_object(values, n);
    }

    Array2D &operator=(const Array2D &a) = delete;
    Array2D &operator=(Array2D &&other) noexcept {
      if (allocator == other.allocator) {
        pstd::swap(extent, other.extent);
        pstd::swap(values, other.values);
      } else if (extent == other.extent) {
        int n = extent.area();
        for (int i = 0; i < n; ++i) {
          allocator.destroy(values + i);
          allocator.construct(values + i, other.values[i]);
        }
        extent = other.extent;
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
      }

      return *this;
    }

    SPECULA_CPU_GPU T &operator[](Point2i p) {
      DASSERT(inside_exclusive(p, extent));
      p.x -= extent.p_min.x;
      p.y -= extent.p_min.y;
      return values[p.x + (extent.p_max.x - extent.p_min.x) * p.y];
    }

    SPECULA_CPU_GPU const T &operator[](Point2i p) const {
      DASSERT(inside_exclusive(p, extent));
      p.x -= extent.p_min.x;
      p.y -= extent.p_min.y;
      return values[p.x + (extent.p_max.x - extent.p_min.x) * p.y];
    }

    SPECULA_CPU_GPU T &operator()(int x, int y) { return (*this)[{x, y}]; }

    SPECULA_CPU_GPU const T &operator()(int x, int y) const { return (*this)[{x, y}]; }

    SPECULA_CPU_GPU [[nodiscard]] int size() const { return extent.area(); }
    SPECULA_CPU_GPU [[nodiscard]] int xsize() const { return extent.p_max.x - extent.p_min.x; }
    SPECULA_CPU_GPU [[nodiscard]] int ysize() const { return extent.p_max.y - extent.p_min.y; }

    SPECULA_CPU_GPU iterator begin() { return values; }
    SPECULA_CPU_GPU const_iterator begin() const { return values; }
    SPECULA_CPU_GPU iterator end() { return begin() + size(); }
    SPECULA_CPU_GPU const_iterator end() const { return begin() + size(); }

    SPECULA_CPU_GPU operator pstd::span<T>() { return pstd::span<T>(values, size()); }
    SPECULA_CPU_GPU operator pstd::span<const T>() const {
      return pstd::span<const T>(values, size());
    }

  private:
    Bounds2i extent;
    Allocator allocator;
    T *values;

    friend struct fmt::formatter<Array2D<T>>;
  };
} // namespace specula

template <typename T> struct fmt::formatter<specula::Array2D<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::Array2D<T> &v, FormatContext &ctx) const {
    format_to(ctx.out(), "[ Array2D extent={} values=[", v.extent);
    for (int y = v.extent.p_min.y; y < v.extent.p_max.y; ++y) {
      format_to(ctx.out(), "[ ");
      for (int x = v.extent.p_min.x; x < v.extent.p_max.x; ++x) {
        format_to(ctx.out(), "{}{}", v(x, y), x < v.extent.p_max.x - 1 ? ", " : " ");
      }
      format_to(ctx.out(), "{}", y < v.extent.p_max.y - 1 ? "], " : "] ");
    }
    return format_to(ctx.out(), "]");
  }
};

#endif // SPECULA_UTIL_CONTAINERS_ARRAY2D_HPP
