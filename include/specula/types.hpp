#ifndef SPECULA_TYPES_HPP
#define SPECULA_TYPES_HPP

namespace specula {
#ifdef SPECULA_FLOAT_AS_DOUBLE
  using Float = double;
#else
  using Float = float;
#endif
} // namespace specula

#endif // SPECULA_TYPES_HPP
