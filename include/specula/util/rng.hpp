#ifndef SPECULA_UTIL_RNG_HPP
#define SPECULA_UTIL_RNG_HPP

#include <cstdint>
#include <limits>
#include <type_traits>

#include <fmt/base.h>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/hash.hpp"

namespace specula {
  static constexpr std::uint64_t PCG32_DEFAULT_STATE = 0x853c49e6748fea9bULL;
  static constexpr std::uint64_t PCG32_DEFAULT_STREAM = 0xda3e39cb94b95bdbULL;
  static constexpr std::uint64_t PCG32_MULT = 0x5851f42d4c957f2dULL;

  class Rng {
  public:
    SPECULA_CPU_GPU Rng() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {}
    SPECULA_CPU_GPU Rng(uint64_t seq_index, uint64_t offset) { set_sequence(seq_index, offset); }
    SPECULA_CPU_GPU Rng(uint64_t seq_index) { set_sequence(seq_index); }

    SPECULA_CPU_GPU void set_sequence(uint64_t sequence_index, uint64_t offset) {
      state = 0u;
      inc = (sequence_index << 1u) | 1u;
      uniform<uint32_t>();
      state += offset;
      uniform<uint32_t>();
    }

    SPECULA_CPU_GPU void set_sequence(uint64_t sequence_index) {
      set_sequence(sequence_index, mix_bits(sequence_index));
    }

    template <typename T> SPECULA_CPU_GPU T uniform() { return T::unimplemented; }

    template <> SPECULA_CPU_GPU inline uint32_t uniform<uint32_t>() {
      uint64_t oldstate = state;
      state = oldstate * PCG32_MULT + inc;
      auto xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
      auto rot = (uint32_t)(oldstate >> 59u);
      return (xorshifted >> rot) | (xorshifted << ((~rot + 1) & 31));
    }

    template <> SPECULA_CPU_GPU inline uint64_t uniform<uint64_t>() {
      uint64_t v0 = uniform<uint32_t>(), v1 = uniform<uint32_t>();
      return (v0 << 32) | v1;
    }

    template <> SPECULA_CPU_GPU inline int32_t uniform<int32_t>() {
      uint32_t v = uniform<uint32_t>();
      if (v <= (uint32_t)std::numeric_limits<int32_t>::max()) {
        return int32_t(v);
      }
      DASSERT_GE(v, (uint32_t)std::numeric_limits<int32_t>::min());
      return int32_t(v - std::numeric_limits<int32_t>::min()) + std::numeric_limits<int32_t>::min();
    }

    template <> SPECULA_CPU_GPU inline int64_t uniform<int64_t>() {
      uint64_t v = uniform<uint64_t>();
      if (v <= (uint64_t)std::numeric_limits<int64_t>::max()) {
        return int64_t(v);
      }
      DASSERT_GE(v, (uint64_t)std::numeric_limits<int64_t>::min());
      return int64_t(v - std::numeric_limits<int64_t>::min()) + std::numeric_limits<int64_t>::min();
    }

    template <> SPECULA_CPU_GPU inline float uniform<float>() {
      return std::min<float>(ONE_MINUS_EPSILON, uniform<uint32_t>() * 0x1p-32f);
    }
    template <> SPECULA_CPU_GPU inline double uniform<double>() {
      return std::min<double>(ONE_MINUS_EPSILON, uniform<uint64_t>() * 0x1p-64);
    }

    template <typename T>
    SPECULA_CPU_GPU T uniform(T b)
      requires(std::is_integral_v<T>)
    {
      T threshold = (~b + 1u) % b;
      while (true) {
        T r = uniform<T>();
        if (r >= threshold) {
          return r % b;
        }
      }
    }

    SPECULA_CPU_GPU void advance(int64_t idelta) {
      uint64_t cur_mult = PCG32_MULT, cur_plus = inc, acc_mult = 1u;
      uint64_t acc_plus = 0u, delta = (uint64_t)idelta;
      while (delta > 0) {
        if ((delta & 1) != 0u) {
          acc_mult *= cur_mult;
          acc_plus = acc_plus * cur_mult + cur_plus;
        }
        cur_plus = (cur_mult + 1) * cur_plus;
        cur_mult *= cur_mult;
        delta /= 2;
      }
      state = acc_mult * state + acc_plus;
    }

    SPECULA_CPU_GPU int64_t operator-(const Rng &other) const {
      ASSERT_EQ(inc, other.inc);
      uint64_t cur_mult = PCG32_MULT, cur_plus = inc, cur_state = other.state;
      uint64_t the_bit = 1u, distance = 0u;
      while (state != cur_state) {
        if ((state & the_bit) != (cur_state & the_bit)) {
          cur_state = cur_state * cur_mult + cur_plus;
          distance |= the_bit;
        }
        ASSERT_EQ(state & the_bit, cur_state & the_bit);
        the_bit <<= 1;
        cur_plus = (cur_mult + 1ULL) * cur_plus;
        cur_mult *= cur_mult;
      }
      return (int64_t)distance;
    }

  private:
    uint64_t state, inc;

    friend struct fmt::formatter<Rng>;
  };

} // namespace specula

template <> struct fmt::formatter<specula::Rng> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext> auto format(const specula::Rng &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ Rng state={} inc={} ]", v.state, v.inc);
  }
};

#endif // SPECULA_UTIL_RNG_HPP
