#ifndef SPECULA_UTIL_PSTD_ARRAY_HPP
#define SPECULA_UTIL_PSTD_ARRAY_HPP

#include <cassert>
#include <cstddef>
#include <initializer_list>

#include "specula/macros.hpp"

namespace specula::pstd {

  /**
   * @brief A container that encapsulates fixed size array
   *
   * @tparam T The element type
   *
   * @see https://en.cppreference.com/cpp/container/array
   */
  template <typename T, std::size_t N> class array;

  /**
   * @brief Specialization of the array class for zero element arrays
   *
   * @tparam T The element type
   */
  template <typename T> class array<T, 0> {
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using size_type = std::size_t;

    array() = default;

    SPECULA_CPU_GPU iterator begin() { return nullptr; }
    SPECULA_CPU_GPU const_iterator begin() const { return nullptr; }
    SPECULA_CPU_GPU const_iterator cbegin() const { return nullptr; }
    SPECULA_CPU_GPU iterator end() { return nullptr; }
    SPECULA_CPU_GPU const_iterator end() const { return nullptr; }
    SPECULA_CPU_GPU const_iterator cend() const { return nullptr; }

    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return 0; }

    SPECULA_CPU_GPU reference operator[](size_type index) {
      assert(!"should never be called");
      static value_type v;
      return v;
    }
    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      assert(!"should never be called");
      return value_type{};
    }

    SPECULA_CPU_GPU pointer data() { return nullptr; }
    SPECULA_CPU_GPU const_pointer data() const { return nullptr; }

    SPECULA_CPU_GPU void fill(const T &v) { assert(!"should never be called"); }

    SPECULA_CPU_GPU bool operator==(const array<T, 0> &other) const { return true; }
    SPECULA_CPU_GPU bool operator!=(const array<T, 0> &other) const { return false; }
  };

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

    array() = default;

    SPECULA_CPU_GPU array(std::initializer_list<T> v) {
      size_type i = 0;
      for (const T &val : v) {
        values[i++] = val;
      }
    }

    SPECULA_CPU_GPU iterator begin() { return values; }
    SPECULA_CPU_GPU const_iterator begin() const { return values; }
    SPECULA_CPU_GPU const_iterator cbegin() const { return values; }
    SPECULA_CPU_GPU iterator end() { return values + N; }
    SPECULA_CPU_GPU const_iterator end() const { return values + N; }
    SPECULA_CPU_GPU const_iterator cend() const { return values + N; }

    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return N; }

    SPECULA_CPU_GPU reference operator[](size_type index) { return values[index]; }
    SPECULA_CPU_GPU const_reference operator[](size_type index) const { return values[index]; }

    SPECULA_CPU_GPU pointer data() { return values; }
    SPECULA_CPU_GPU const_pointer data() const { return values; }

    SPECULA_CPU_GPU void fill(const T &v) {
      for (size_type i = 0; i < N; ++i) {
        values[i] = v;
      }
    }

    SPECULA_CPU_GPU bool operator==(const array<T, N> &other) const {
      for (size_type i = 0; i < N; ++i) {
        if (values[i] != other.values[i]) {
          return false;
        }
      }
      return true;
    }
    SPECULA_CPU_GPU bool operator!=(const array<T, N> &other) const { return !(*this == other); }

  private:
    T values[N] = {};
  };
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_ARRAY_HPP
