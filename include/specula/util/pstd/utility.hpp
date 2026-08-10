#ifndef SPECULA_UTIL_PSTD_UTILITY_HPP
#define SPECULA_UTIL_PSTD_UTILITY_HPP

#include <cstring>
#include <type_traits>
#include <utility>

#include "specula/macros.hpp"

namespace specula::pstd {
  template <typename T> SPECULA_CPU_GPU inline void swap(T &a, T &b) noexcept {
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
  }

  template <class To, class From>
  SPECULA_CPU_GPU To bit_cast(const From &src) noexcept
    requires(sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&
             std::is_trivially_copyable_v<To> && std::is_trivially_constructible_v<To>)
  {
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
  }
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_UTILITY_HPP
