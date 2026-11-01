/**
 * @file
 * @brief Device-safe fixed-size array, a stand-in for `std::array`.
 */

#ifndef SPECULA_UTIL_PSTD_ARRAY_HPP
#define SPECULA_UTIL_PSTD_ARRAY_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cassert>
#include <cstddef>
#include <initializer_list>

#include "specula/macros.hpp"

namespace specula::pstd {

  /**
   * @brief Fixed-size array of `N` elements of `T`, usable on both CPU and GPU.
   *
   * Port of pbrt's `pstd::array`: `std::array` cannot be relied on in GPU device code. Unlike
   * `std::array` it is not an aggregate: it is built from an `std::initializer_list` and its
   * elements are value-initialized otherwise, so `T` must be default constructible. Only the subset
   * of the `std::array` interface that pbrt uses is provided (no `at`, `front`, `back`, `empty`,
   * reverse iterators, or ordering comparisons), and nothing is `constexpr`.
   *
   * @see https://en.cppreference.com/w/cpp/container/array
   */
  template <typename T, std::size_t N> class array {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = std::size_t;

    /** @brief Value-initializes every element. */
    array() = default;

    /**
     * @brief Copies the elements of `v` into the front of the array.
     *
     * Elements past `v.size()` keep their value-initialized state.
     *
     * @pre `v.size() <= N`; this is not checked.
     * @todo Writing past `N` when `v.size() > N` is undefined behaviour; add a `DASSERT`.
     */
    SPECULA_CPU_GPU array(std::initializer_list<T> v) {
      size_type i = 0;
      for (const T &val : v) {
        values[i++] = val;
      }
    }

    /** @brief Unchecked element access. @pre `index < N`. */
    SPECULA_CPU_GPU reference operator[](size_type index) { return values[index]; }
    /** @brief Unchecked element access. @pre `index < N`. */
    SPECULA_CPU_GPU const_reference operator[](size_type index) const { return values[index]; }

    /** @brief Pointer to the first element. */
    SPECULA_CPU_GPU pointer data() { return values; }
    /** @brief Pointer to the first element. */
    SPECULA_CPU_GPU const_pointer data() const { return values; }

    /** @brief Iterator to the first element. */
    SPECULA_CPU_GPU iterator begin() { return values; }
    /** @brief Iterator to the first element. */
    SPECULA_CPU_GPU const_iterator begin() const { return values; }
    /** @brief Iterator one past the last element. */
    SPECULA_CPU_GPU iterator end() { return values + N; }
    /** @brief Iterator one past the last element. */
    SPECULA_CPU_GPU const_iterator end() const { return values + N; }
    /** @brief Const iterator to the first element. */
    SPECULA_CPU_GPU const_iterator cbegin() const { return values; }
    /** @brief Const iterator one past the last element. */
    SPECULA_CPU_GPU const_iterator cend() const { return values + N; }

    /** @brief Number of elements, always `N`. */
    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return N; }

    /** @brief Element-wise equality; requires `T` to provide `operator!=`. */
    SPECULA_CPU_GPU bool operator==(const array<T, N> &other) const {
      for (size_type i = 0; i < N; ++i) {
        if (values[i] != other.values[i]) {
          return false;
        }
      }
      return true;
    }
    /** @brief Negation of `operator==`. */
    SPECULA_CPU_GPU bool operator!=(const array<T, N> &other) const { return !(*this == other); }

    /** @brief Assigns `v` to every element. */
    SPECULA_CPU_GPU void fill(const T &v) {
      for (size_type i = 0; i < N; ++i) {
        values[i] = v;
      }
    }

  private:
    T values[N] = {};
  };

  /**
   * @brief Empty array specialization; avoids declaring the ill-formed `T values[0]`.
   *
   * Iterators and `data()` are `nullptr`, so `begin() == end()` and range-for loops do nothing.
   * Element access and `fill()` trip an `assert` because they should never be reached.
   *
   * @warning `fill()` asserts, whereas `std::array<T, 0>::fill` is a valid no-op.
   */
  template <typename T> class array<T, 0> {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = std::size_t;

    /** @brief Constructs the empty array. */
    array() = default;

    /** @brief Must not be called; returns a reference to a function-local static. */
    SPECULA_CPU_GPU reference operator[](size_type index) {
      assert(!"should never be called");
      static value_type v;
      return v;
    }
    /**
     * @brief Must not be called.
     * @todo Returns a reference to a temporary (dangling, UB); pbrt returns a static like the
     *       non-const overload.
     */
    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      assert(!"should never be called");
      return value_type{};
    }

    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU pointer data() { return nullptr; }
    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU const_pointer data() const { return nullptr; }

    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU iterator begin() { return nullptr; }
    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU const_iterator begin() const { return nullptr; }
    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU iterator end() { return nullptr; }
    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU const_iterator end() const { return nullptr; }
    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU const_iterator cbegin() const { return nullptr; }
    /** @brief Always `nullptr`. */
    SPECULA_CPU_GPU const_iterator cend() const { return nullptr; }

    /** @brief Always 0. */
    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return 0; }

    /** @brief Always `true`: two empty arrays are equal. */
    SPECULA_CPU_GPU bool operator==(const array<T, 0> &other) const { return true; }
    /** @brief Always `false`. */
    SPECULA_CPU_GPU bool operator!=(const array<T, 0> &other) const { return false; }

    /** @brief Must not be called (asserts). */
    SPECULA_CPU_GPU void fill(const T &v) { assert(!"should never be called"); }
  };
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_ARRAY_HPP
