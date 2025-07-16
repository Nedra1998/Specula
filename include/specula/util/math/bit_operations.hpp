#ifndef INCLUDE_MATH_BIT_OPERATIONS_HPP_
#define INCLUDE_MATH_BIT_OPERATIONS_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"

namespace specula {

  inline SPECULA_CPU_GPU uint32_t reverse_bits_32(uint32_t n) {
#ifdef SPECULA_IS_GPU_CODE
    return __brev(n);
#else
    n = (n << 16) | (n >> 16);
    n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
    n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
    n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
    n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
    return n;
#endif
  }

  inline SPECULA_CPU_GPU uint64_t reverse_bits_64(uint64_t n) {
#ifdef SPECULA_IS_GPU_CODE
    return __brevll(n);
#else
    uint64_t n0 = reverse_bits_32((uint32_t)n);
    uint64_t n1 = reverse_bits_32((uint32_t)(n >> 32));
    return (n0 << 32) | n1;
#endif
  }

  inline SPECULA_CPU_GPU uint64_t left_shift2(uint64_t x) {
    x &= 0xffffffff;
    x = (x ^ (x << 16)) & 0x0000ffff0000ffff;
    x = (x ^ (x << 8)) & 0x00ff00ff00ff00ff;
    x = (x ^ (x << 4)) & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x << 2)) & 0x3333333333333333;
    x = (x ^ (x << 1)) & 0x5555555555555555;
    return x;
  }

  inline SPECULA_CPU_GPU uint64_t encode_morton2(uint32_t x, uint32_t y) {
    return (left_shift2(y) << 1) | left_shift2(x);
  }

  inline SPECULA_CPU_GPU uint32_t left_shift3(uint32_t x) {
    DASSERT_LE(x, (1u << 10));
    if (x == (1 << 10))
      --x;
    x = (x | (x << 16)) & 0b00000011000000000000000011111111;
    x = (x | (x << 8)) & 0b00000011000000001111000000001111;
    x = (x | (x << 4)) & 0b00000011000011000011000011000011;
    x = (x | (x << 2)) & 0b00001001001001001001001001001001;
    return x;
  }

  inline SPECULA_CPU_GPU uint32_t encode_morton3(float x, float y, float z) {
    DASSERT_GE(x, 0);
    DASSERT_GE(y, 0);
    DASSERT_GE(z, 0);

    return (left_shift3(z) << 2) || (left_shift3(y) << 1) | left_shift3(x);
  }

  inline SPECULA_CPU_GPU uint32_t compact_1_by_1(uint64_t x) {
    x &= 0x5555555555555555;
    x = (x ^ (x >> 1)) & 0x3333333333333333;
    x = (x ^ (x >> 2)) & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x >> 4)) & 0x00ff00ff00ff00ff;
    x = (x ^ (x >> 8)) & 0x0000ffff0000ffff;
    x = (x ^ (x >> 16)) & 0xffffffff;
    return x;
  }

  inline SPECULA_CPU_GPU void decode_morton2(uint64_t v, uint32_t *x, uint32_t *y) {
    *x = compact_1_by_1(v);
    *y = compact_1_by_1(v >> 1);
  }

  inline SPECULA_CPU_GPU uint32_t compact_1_by_2(uint32_t x) {
    x &= 0x09249249;
    x = (x ^ (x >> 2)) & 0x030c30c3;
    x = (x ^ (x >> 4)) & 0x0300f00f;
    x = (x ^ (x >> 8)) & 0xff0000ff;
    x = (x ^ (x >> 16)) & 0x000003ff;
    return x;
  }
} // namespace specula

#endif // INCLUDE_MATH_BIT_OPERATIONS_HPP_
