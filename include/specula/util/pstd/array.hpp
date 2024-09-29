/**
 * @file array.hpp
 * @brief Portable array class
 *
 * This file provides a portable array class that is available on both the host and device, to allow
 * the code to be compiled for both CPU and GPU platforms.
 */

#ifndef INCLUDE_PSTD_ARRAY_HPP_
#define INCLUDE_PSTD_ARRAY_HPP_

#include <cstddef>

#include "specula/specula.hpp"

namespace specula::pstd {
  /**
   * @class array
   * @brief Fixed-sized inplace contiguous array
   *
   * @tparam T The element type of the array
   * @see https://en.cppreference.com/w/cpp/container/array
   */
  template <typename T, std::size_t N> class array;

  /**
   * @brief Specialization of the array class for zero-sized arrays
   *
   * @tparam T The element type of the array
   */
  template <typename T> class array<T, 0> {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using size_t = std::size_t;

    /// @brief Default constructor
    array() = default;

    /**
     * @brief Fill the container with specified value
     *
     * @param v Value to fill the container with
     *
     * @warning This function should never be called as the array is zero-sized, so there is no
     * memory to fill. An assertion is triggered if this function is called.
     */
    SPECULA_CPU_GPU void fill(const T &v) { assert(!"should never be called"); }

    SPECULA_CPU_GPU bool operator==(const array<T, 0> &a) const { return true; }
    SPECULA_CPU_GPU bool operator!=(const array<T, 0> &a) const { return false; }

    /// @brief Returns an iterator to the beginning
    SPECULA_CPU_GPU iterator begin() { return nullptr; }
    /// @brief Returns an iterator to the beginning
    SPECULA_CPU_GPU const_iterator begin() const { return nullptr; }

    /// @brief Returns an iterator to the end
    SPECULA_CPU_GPU iterator end() { return nullptr; }
    /// @brief Returns an iterator to the end
    SPECULA_CPU_GPU const_iterator end() const { return nullptr; }

    /// @brief Returns the number of elements
    SPECULA_CPU_GPU size_t size() const { return 0; }

    /// @brief Access specified element
    SPECULA_CPU_GPU T &operator[](size_t i) {
      assert(!"should never be called");
      static T t;
      return t;
    }
    /// @brief Access specified element
    SPECULA_CPU_GPU T operator[](size_t i) const {
      assert(!"should never be called");
      return T{};
    }

    /// @brief Direct access to the underlying contiguous storage
    SPECULA_CPU_GPU T *data() { return nullptr; }
    /// @brief Direct access to the underlying contiguous storage
    SPECULA_CPU_GPU const T *data() const { return nullptr; }
  };

  template <typename T, std::size_t N> class array {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using size_t = std::size_t;

    /// @brief Default constructor
    array() = default;

    /// @brief Construct the array from an initializer list
    SPECULA_CPU_GPU array(std::initializer_list<T> v) {
      size_t i = 0;
      for (const T &value : v)
        values[i++] = value;
    }

    /// @brief Fill the container with specified value
    SPECULA_CPU_GPU void fill(const T &v) {
      for (size_t i = 0; i < N; ++i)
        values[i] = v;
    }

    SPECULA_CPU_GPU bool operator==(const array<T, 0> &a) const {
      for (size_t i = 0; i < N; ++i)
        if (values[i] != a.values[i])
          return false;
      return true;
    }

    SPECULA_CPU_GPU bool operator!=(const array<T, 0> &a) const { return !(*this == a); }

    /// @brief Returns an iterator to the beginning
    SPECULA_CPU_GPU iterator begin() { return values; }
    /// @brief Returns an iterator to the beginning
    SPECULA_CPU_GPU const_iterator begin() const { return values; }

    /// @brief Returns an iterator to the end
    SPECULA_CPU_GPU iterator end() { return values + N; }
    /// @brief Returns an iterator to the end
    SPECULA_CPU_GPU const_iterator end() const { return values + N; }

    /// @brief Returns the number of elements
    SPECULA_CPU_GPU size_t size() const { return N; }

    /// @brief Access specified element
    SPECULA_CPU_GPU T &operator[](size_t i) { return values[i]; }
    /// @brief Access specified element
    SPECULA_CPU_GPU T operator[](size_t i) const { return values[i]; }

    /// @brief Direct access to the underlying contiguous storage
    SPECULA_CPU_GPU T *data() { return values; }
    /// @brief Direct access to the underlying contiguous storage
    SPECULA_CPU_GPU const T *data() const { return values; }

  private:
    T values[N] = {};
  };
} // namespace specula::pstd

#endif // INCLUDE_PSTD_ARRAY_HPP_
