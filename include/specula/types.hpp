#ifndef SPECULA_TYPES_HPP
#define SPECULA_TYPES_HPP

#include "specula/util/pstd/pmr.hpp"

namespace specula {
#ifdef SPECULA_FLOAT_AS_DOUBLE
  using Float = double;
#else
  using Float = float;
#endif

  using Allocator = pstd::pmr::polymorphic_allocator<std::byte>;
} // namespace specula

#endif // SPECULA_TYPES_HPP
