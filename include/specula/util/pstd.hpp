#ifndef SPECULA_UTIL_PSTD_HPP
#define SPECULA_UTIL_PSTD_HPP

#include <type_traits>

#include "specula/specula.hpp"

namespace specula::pstd {

  template <class To, class From>
  SPECULA_CPU_GPU
      typename std::enable_if_t<sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From> &&
                                    std::is_trivially_copyable_v<To>,
                                To>
      bit_cast(const From &src) noexcept {
    static_assert(
        std::is_trivially_constructible_v<To>,
        "This implementation requires the destination type to be trivially constructable");
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
  }

} // namespace specula::pstd

#endif // !SPECULA_UTIL_PSTD_HPP
