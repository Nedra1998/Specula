#ifndef SPECULA_TYPES_HPP
#define SPECULA_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace specula {
#ifdef SPECULA_FLOAT_AS_DOUBLE
  using Float = double;
  using FloatBits = uint64_t;
#else
  using Float = float;
  using FloatBits = uint32_t;
#endif

  namespace pstd::pmr {
    template <typename T> class polymorphic_allocator;
  }

  using Allocator = pstd::pmr::polymorphic_allocator<std::byte>;
} // namespace specula

#endif // SPECULA_TYPES_HPP
