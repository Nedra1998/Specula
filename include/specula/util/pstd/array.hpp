#ifndef INCLUDE_PSTD_ARRAY_HPP_
#define INCLUDE_PSTD_ARRAY_HPP_

#include <cstddef>

#include "specula/specula.hpp"

namespace specula::pstd {
  template <typename T, std::size_t N> class array;

  template <typename T> class array<T, 0> {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using size_t = std::size_t;

    array() = default;

    SPECULA_CPU_GPU void fill(const T &v) { assert(!"should never be called"); }

    SPECULA_CPU_GPU bool operator==(const array<T, 0> &a) const { return true; }
    SPECULA_CPU_GPU bool operator!=(const array<T, 0> &a) const { return false; }

    SPECULA_CPU_GPU iterator begin() { return nullptr; }
    SPECULA_CPU_GPU iterator end() { return nullptr; }
    SPECULA_CPU_GPU const_iterator begin() const { return nullptr; }
    SPECULA_CPU_GPU const_iterator end() const { return nullptr; }

    SPECULA_CPU_GPU size_t size() const { return 0; }

    SPECULA_CPU_GPU T &operator[](size_t i) {
      assert(!"should never be called");
      static T t;
      return t;
    }
    SPECULA_CPU_GPU T operator[](size_t i) const {
      assert(!"should never be called");
      return T{};
    }

    SPECULA_CPU_GPU T *data() { return nullptr; }
    SPECULA_CPU_GPU const T *data() const { return nullptr; }
  };

  template <typename T, std::size_t N> class array {
  public:
    using value_type = T;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using size_t = std::size_t;

    array() = default;

    SPECULA_CPU_GPU array(std::initializer_list<T> v) {
      size_t i = 0;
      for (const T &value : v)
        values[i++] = value;
    }

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

    SPECULA_CPU_GPU iterator begin() { return values; }
    SPECULA_CPU_GPU iterator end() { return values + N; }
    SPECULA_CPU_GPU const_iterator begin() const { return values; }
    SPECULA_CPU_GPU const_iterator end() const { return values + N; }

    SPECULA_CPU_GPU size_t size() const { return N; }

    SPECULA_CPU_GPU T &operator[](size_t i) { return values[i]; }
    SPECULA_CPU_GPU T operator[](size_t i) const { return values[i]; }

    SPECULA_CPU_GPU T *data() { return values; }
    SPECULA_CPU_GPU const T *data() const { return values; }

  private:
    T values[N] = {};
  };
} // namespace specula::pstd

#endif // INCLUDE_PSTD_ARRAY_HPP_
