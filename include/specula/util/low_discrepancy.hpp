#ifndef SPECULA_UTIL_LOW_DISCREPANCY_HPP
#define SPECULA_UTIL_LOW_DISCREPANCY_HPP

#include <cstdint>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/hash.hpp"
#include "specula/util/math.hpp"
#include "specula/util/math/bit_operations.hpp"
#include "specula/util/primes.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/sobolmatrices.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  enum class RandomizeStrategy : uint8_t { NONE, PERMUTE_DIGITS, FAST_OWEN, OWEN };

  class DigitPermuation {
  public:
    DigitPermuation() = default;
    DigitPermuation(int base, uint32_t seed, Allocator alloc) : base(base) {
      ASSERT_LT(base, 65536);
      n_digits = 0;
      Float inv_base = (Float)1 / (Float)base, inv_base_m = 1;
      while (1 - (base - 1) * inv_base_m < 1) {
        ++n_digits;
        inv_base_m *= inv_base;
      }

      permutations = alloc.allocate_object<uint16_t>(n_digits * base);
      for (int digit_index = 0; digit_index < n_digits; ++digit_index) {
        uint64_t dseed = hash(base, digit_index, seed);
        for (int digit_value = 0; digit_value < base; ++digit_value) {
          int index = digit_index * base + digit_value;
          permutations[index] = permutation_element(digit_value, base, dseed);
        }
      }
    }

    SPECULA_CPU_GPU [[nodiscard]] int permute(int digit_index, int digit_value) const {
      DASSERT_LT(digit_index, n_digits);
      DASSERT_LT(digit_value, base);
      return permutations[digit_index * base + digit_value];
    }

  private:
    int base, n_digits;
    uint16_t *permutations;

    friend fmt::formatter<DigitPermuation>;
  };

  struct NoRandomizer {
    SPECULA_CPU_GPU uint32_t operator()(uint32_t v) const { return v; }
  };

  struct BinaryPermuteScrambler {
    SPECULA_CPU_GPU BinaryPermuteScrambler(uint32_t perm) : permutation(perm) {}

    SPECULA_CPU_GPU uint32_t operator()(uint32_t v) const { return permutation ^ v; }

    uint32_t permutation;
  };

  struct FastOwenScrambler {
    SPECULA_CPU_GPU FastOwenScrambler(uint32_t seed) : seed(seed) {}

    SPECULA_CPU_GPU uint32_t operator()(uint32_t v) const {
      v = reverse_bits_32(v);
      v ^= v * 0x3d20adea;
      v += seed;
      v *= (seed >> 16) | 1;
      v ^= v * 0x05526c56;
      v ^= v * 0x53a22864;
      return reverse_bits_32(v);
    }

    uint32_t seed;
  };

  struct OwenScrambler {
    SPECULA_CPU_GPU OwenScrambler(uint32_t seed) : seed(seed) {}

    SPECULA_CPU_GPU uint32_t operator()(uint32_t v) const {
      if ((seed & 1) != 0u) {
        v ^= 1u << 31;
      }
      for (int b = 1; b < 32; ++b) {
        uint32_t mask = (~0u) << (32 - b);
        if ((uint32_t)mix_bits((v & mask) ^ seed) & (1u << b)) {
          v ^= 1u << (31 - b);
        }
      }
      return v;
    }

    uint32_t seed;
  };

  SPECULA_CPU_GPU inline uint64_t sobol_interval_to_index(uint32_t m, uint64_t frame, Point2i p) {
    if (m == 0) {
      return frame;
    }

    const uint32_t m2 = m << 1;
    uint64_t index = frame << m2;
    uint64_t delta = 0;
    for (int c = 0; frame != 0u; frame >>= 1, ++c) {
      if ((frame & 1) != 0u) {
        delta ^= VDC_SOBOL_MATRICES[m - 1][c];
      }
    }

    uint64_t b = (((uint64_t)((uint32_t)p.x) << m) | ((uint32_t)p.y)) ^ delta;

    for (int c = 0; b != 0u; b >>= 1, ++c) {
      if ((b & 1) != 0u) {
        index ^= VDC_SOBOL_MATRICES[m - 1][c];
      }
    }

    return index;
  }

  SPECULA_CPU_GPU inline Float blue_noise_sample(Point2i p, int instance) {
    auto hash_perm = [&](uint64_t index) -> int {
      return uint32_t(mix_bits(index ^ (0x55555555 * instance)) >> 24) % 24;
    };

    int n_base4_digits = 8;
    p.x &= 255;
    p.y &= 255;
    uint64_t morton_index = encode_morton2(p.x, p.y);

    static const uint8_t permutations[24][4] = {
        {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 2, 1, 3}, {0, 2, 3, 1}, {0, 3, 2, 1}, {0, 3, 1, 2},
        {1, 0, 2, 3}, {1, 0, 3, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 2, 0}, {1, 3, 0, 2},
        {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 0, 1, 3}, {2, 0, 3, 1}, {2, 3, 0, 1}, {2, 3, 1, 0},
        {3, 1, 2, 0}, {3, 1, 0, 2}, {3, 2, 1, 0}, {3, 2, 0, 1}, {3, 0, 2, 1}, {3, 0, 1, 2}};

    uint32_t sample_index = 0;
    for (int i = n_base4_digits - 1; i >= 0; --i) {
      int digit_shift = 2 * i;
      int digit = (morton_index >> digit_shift) & 3;
      int p = hash_perm(morton_index >> (digit_shift + 2));
      digit = permutations[p][digit];
      sample_index |= digit << digit_shift;
    }

    return Float(reverse_bits_32(sample_index)) * 0x1p-32f;
  }

  SPECULA_CPU_GPU inline Float radical_inverse(int base_index, uint64_t a) {
    unsigned int base = PRIMES[base_index];

    uint64_t limit = ~0ull / base - base;
    Float inv_base = (Float)1 / (Float)base, inv_base_m = 1;
    uint64_t reversed_digits = 0;
    while ((a != 0u) && reversed_digits < limit) {
      uint64_t next = a / base;
      uint64_t digit = a - next * base;
      reversed_digits = reversed_digits * base + digit;
      inv_base_m *= inv_base;
      a = next;
    }
    return std::min<Float>(Float(reversed_digits) * inv_base_m, ONE_MINUS_EPSILON);
  }

  SPECULA_CPU_GPU inline Float scrambled_radical_inverse(int base_index, uint64_t a,
                                                         const DigitPermuation &perm) {
    unsigned int base = PRIMES[base_index];

    uint64_t limit = ~0ull / base - base;
    Float inv_base = (Float)1 / (Float)base, inv_base_m = 1;
    uint64_t reversed_digits = 0;
    int digit_index = 0;
    while (1 - (base - 1) * inv_base_m < 1 && reversed_digits < limit) {
      uint64_t next = a / base;
      int digit_value = a - next * base;
      reversed_digits = reversed_digits * base + perm.permute(digit_index, digit_value);
      inv_base_m *= inv_base;
      ++digit_index;
      a = next;
    }
    return std::min<Float>(Float(reversed_digits) * inv_base_m, ONE_MINUS_EPSILON);
  }

  SPECULA_CPU_GPU inline uint64_t inverse_radical_inverse(uint64_t inverse, int base,
                                                          int n_digits) {
    uint64_t index = 0;
    for (int i = 0; i < n_digits; ++i) {
      uint64_t digit = inverse % base;
      inverse /= base;
      index = index * base + digit;
    }
    return index;
  }

  SPECULA_CPU_GPU inline Float owen_scrambled_radical_inverse(int base_index, uint64_t a,
                                                              uint32_t hash) {
    unsigned int base = PRIMES[base_index];

    uint64_t limit = ~0ull / base - base;
    Float inv_base = (Float)1 / (Float)base, inv_base_m = 1;
    uint64_t reversed_digits = 0;
    int digit_index = 0;
    while (1 - inv_base_m < 1 && reversed_digits < limit) {
      uint64_t next = a / base;
      int digit_value = a - next * base;
      uint32_t digit_hash = mix_bits(hash ^ reversed_digits);
      digit_value = permutation_element(digit_value, base, digit_hash);
      reversed_digits = reversed_digits * base + digit_value;
      inv_base_m *= inv_base;
      ++digit_index;
      a = next;
    }
    return std::min<Float>(Float(reversed_digits) * inv_base_m, ONE_MINUS_EPSILON);
  }

  SPECULA_CPU_GPU inline uint32_t multiply_generator(pstd::span<const uint32_t> c, uint32_t a) {
    uint32_t v = 0;
    for (int i = 0; a != 0; ++i, a >>= 1) {
      if ((a & 1) != 0u) {
        v ^= c[i];
      }
    }
    return v;
  }

  template <typename R>
  SPECULA_CPU_GPU inline Float sobol_sample(int64_t a, int dimension, R randomizer) {
    DASSERT_LT(dimension, N_SOBOL_DIMENSIONS);
    DASSERT(a >= 0 && a < (1ull << SOBOL_MATRIX_SIZE));

    uint32_t v = 0;
    for (size_t i = dimension * SOBOL_MATRIX_SIZE; a != 0; a >>= 1, ++i) {
      if (a & 1) {
        v ^= SOBOL_MATRICES_32[i];
      }
    }

    v = randomizer(v);
    return std::min(Float(v) * 0x1p-32f, FLOAT_ONE_MINUS_EPSILON);
  }

  pstd::vector<DigitPermuation> *compute_radical_inverse_permutations(uint32_t seed,
                                                                      Allocator alloc = {});
} // namespace specula

template <> struct fmt::formatter<specula::DigitPermuation> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::DigitPermuation &v, FormatContext &ctx) const {
    format_to(ctx.out(), "[ DigitPermutation base={} nDigits={} permutations={{", v.base,
              v.n_digits);
    for (int digit_index = 0; digit_index < v.n_digits; ++digit_index) {
      format_to(ctx.out(), "{}=(", digit_index);
      for (int digit_value = 0; digit_value < v.base; ++digit_value) {
        format_to(ctx.out(), "{}{}", v.permutations[digit_index * v.base + digit_value],
                  digit_value != v.base - 1 ? ", " : "");
      }
      format_to(ctx.out(), ") ");
    }

    return format_to(ctx.out(), "}} ]");
  }
};

template <> struct fmt::formatter<specula::RandomizeStrategy> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RandomizeStrategy &v, FormatContext &ctx) const {
    switch (v) {
    case specula::RandomizeStrategy::NONE:
      return format_to(ctx.out(), "None");
    case specula::RandomizeStrategy::PERMUTE_DIGITS:
      return format_to(ctx.out(), "PermtueDigits");
    case specula::RandomizeStrategy::FAST_OWEN:
      return format_to(ctx.out(), "FastOwen");
    case specula::RandomizeStrategy::OWEN:
      return format_to(ctx.out(), "Owen");
    }
  }
};

#endif // SPECULA_UTIL_LOW_DISCREPANCY_HPP
