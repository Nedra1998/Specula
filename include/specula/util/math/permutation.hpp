#ifndef SPECULA_UTIL_MATH_PERMUTATION_HPP
#define SPECULA_UTIL_MATH_PERMUTATION_HPP

#include <cstdint>

#include "specula/macros.hpp"
namespace specula {
  SPECULA_CPU_GPU inline uint32_t permutation_element(uint32_t i, uint32_t l, uint32_t p) {
    uint32_t w = l - 1;
    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;
    do {
      i ^= p;
      i *= 0xe170893d;
      i ^= p >> 16;
      i ^= (i & w) >> 4;
      i ^= p >> 8;
      i *= 0x0929eb3f;
      i ^= p >> 23;
      i ^= (i & w) >> 1;
      i *= 1 | p >> 27;
      i *= 0x6935fa69;
      i ^= (i & w) >> 11;
      i *= 0x74dcb303;
      i ^= (i & w) >> 2;
      i *= 0x9e501cc3;
      i ^= (i & w) >> 2;
      i *= 0xc860a3df;
      i &= w;
      i ^= i >> 5;
    } while (i >= l);
    return (i + p) % l;
  }
} // namespace specula

#endif // SPECULA_UTIL_MATH_PERMUTATION_HPP
