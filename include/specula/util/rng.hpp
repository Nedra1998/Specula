/**
 * @file rng.hpp
 * @brief High-performance, pseudo-random number generator (PRNG) based on PCG32
 *
 * This file provides an implementation of the PCG32 pseudo-random number generator (PRNG), which is
 * a high-performance, statistically robust PRNG designed to be simple and efficient. PCG32 uses a
 * Linear Congruential Generator (LCG) as its core, combined with a permutation step that scrambles
 * the bits of the output to improve the quality of randomness. The generator is capable of
 * producing 32-bit random numbers and offers features like multiple independent streams of random
 * numbers and a variety of output functions, ensuring versatility across different use cases.
 */

#ifndef SPECULA_UTIL_RNG_HPP_
#define SPECULA_UTIL_RNG_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/hash.hpp"

namespace specula {

  /// Default state value for PCG32
  static constexpr std::uint64_t PCG32_DEFAULT_STATE = 0x853c49e6748fea9bULL;

  /// Default increment value for PCG32
  static constexpr std::uint64_t PCG32_DEFAULT_STREAM = 0xda3e39cb94b95bdbULL;

  /// Multiplier value for PCG32
  static constexpr std::uint64_t PCG32_MULT = 0x5851f42d4c957f2dULL;

  /**
   * @class Rng
   * @brief An implementation of the PCG32 pseudo-random number generator
   *
   * PCG32 is a high-performance, pseudo-random number generator (PRNG) designed to be simple,
   * efficient, and statistically robust. It uses a Linear Congruential Generator (LCG) as its core,
   * combined with a permutation step that scrambles the bits of the output to improve the quality
   * of randomness. PCG32 generates 32-bit random numbers and is notable for its ability to provide
   * excellent randomness with small state size, making it memory efficient. It also offers features
   * like multiple independent streams of random numbers and a variety of output functions, ensuring
   * versatility across different use cases. Despite being fast and lightweight, PCG32 maintains
   * high statistical quality, outperforming many traditional PRNGs like the Mersenne Twister in
   * terms of both speed and randomness.
   *
   * This RNG implementation allows users to provide two 64-bit values to configure its operation:
   *
   * - one choses from one of \f(2^{63}\f) different sequences of \f(2^{64}\f) random numbers.
   * - the second effectivly selects the starting point within that sequence.
   *
   * Most other random number generators only support the second, which is not as useful. Having
   * independent non-overlaping sequences of values rather than different starting points in the
   * same sequence provides greater nonuniformity in the generated values.
   *
   * @note In general usages of the `Rng` class should always provide an initial sequence index to
   * avoid different parts of the system inadvertenly using correlated sequences of pseudo-random
   * values.
   *
   * @see https://en.wikipedia.org/wiki/Permuted_congruential_generator
   */
  class Rng {
  public:
    /**
     * @brief Default constructor for PCG32 with default state and stream.
     *
     * Initializes the generator with default state and increment.
     */
    SPECULA_CPU_GPU Rng() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) {}

    /**
     * @brief Constructs a PCG32 generator with a specific sequence index and offset.
     *
     * @param seq_index Sequence index to initialize the generator.
     * @param offset Offset to adjust the initial state.
     */
    SPECULA_CPU_GPU Rng(std::uint64_t seq_index, std::uint64_t offset) {
      set_sequence(seq_index, offset);
    }

    /**
     * @brief Constructs a PCG32 generator with a specific sequence index.
     *
     * @param seq_index Sequence index to initialize the generator.
     */
    SPECULA_CPU_GPU Rng(std::uint64_t seq_index) { set_sequence(seq_index); }

    /**
     * @brief Sets the generator's sequence with a specific index and offset.
     *
     * The state is initialized to zero, and the increment is derived from the sequence index.
     * The generator is "warmed up" by calling `uniform()` twice to advance the state.
     *
     * @param seq_index Sequence index used to determine the increment.
     * @param offset Offset added to the initial state.
     */
    SPECULA_CPU_GPU inline void set_sequence(std::uint64_t seq_index, std::uint64_t offset);

    /**
     * @brief Sets the generator's sequence using the sequence index only.
     *
     * The offset is derived by hashing the sequence index.
     *
     * @param seq_index Sequence index used to determine the increment and hashed offset.
     */
    SPECULA_CPU_GPU inline void set_sequence(std::uint64_t seq_index) {
      set_sequence(seq_index, mix_bits(seq_index));
    }

    /**
     * @brief Generates a random value of type T.
     *
     * @warning This function is not implemented for all types and will throw an assertion error if
     * called with an unsupported type.
     *
     * @tparam T Type of the random value to generate.
     * @return Random value of type T.
     */
    template <typename T> SPECULA_CPU_GPU T uniform();

    /**
     * @brief Generates a random integer in the range \f$[0, b - 1]\f$.
     *
     * This method performs rejection sampling to ensure uniformity.
     *
     * @tparam T The type of the integer.
     * @param b Upper bound (exclusive) for the generated random number.
     * @return A random integer in the range \f$[0, b - 1]\f$.
     */
    template <typename T>
    SPECULA_CPU_GPU typename std::enable_if_t<std::is_integral_v<T>, T> uniform(T b) {
      T threshold = (~b + 1u) % b;
      while (true) {
        T r = uniform<T>();
        if (r >= threshold)
          return r % b;
      }
    }

    /**
     * @brief Advances the internal state of the generator by a given delta.
     *
     * The state is advanced using exponentiation by squaring, which allows for efficient
     * large jumps in the random number sequence, without the need to generate intermediate
     * values.
     *
     * @param idelta Number of steps to advance.
     */
    SPECULA_CPU_GPU inline void advance(std::int64_t idelta) {
      std::uint64_t cur_mult = PCG32_MULT, cur_plus = inc, acc_mult = 1u, acc_plus = 0u,
                    delta = (std::uint64_t)idelta;
      while (delta > 0) {
        if (delta & 1) {
          acc_mult *= cur_mult;
          acc_plus = acc_plus * cur_mult + cur_plus;
        }
        cur_plus = (cur_mult + 1) * cur_plus;
        cur_mult *= cur_mult;
        delta /= 2;
      }
      state = acc_mult * state + acc_plus;
    }

    /**
     * @brief Computes the difference in steps between two RNG states.
     *
     * This method determines how many steps separate the state of the current generator
     * from another generator, assuming they share the same sequence.
     *
     * @param other The other RNG to compare against.
     * @return The number of steps between the two states.
     */
    SPECULA_CPU_GPU inline std::int64_t operator-(const Rng &other) const {
      ASSERT_EQ(inc, other.inc);
      std::uint64_t cur_mult = PCG32_MULT, cur_plus = inc, cur_state = other.state, the_bit = 1u,
                    distance = 0u;
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

      return (std::int64_t)distance;
    }

  private:
    std::uint64_t state, inc;
  };

  template <typename T> inline T Rng::uniform() { return T::unimplemented; }

  /**
   * @brief Generates a 32-bit unsigned integer.
   *
   * The state is updated using the linear congruential formula, and the output is
   * derived by applying an XOR-shift and rotation, ensuring randomness.
   *
   * @return A 32-bit unsigned integer.
   */
  template <> inline std::uint32_t Rng::uniform<std::uint32_t>() {
    std::uint64_t oldstate = state;
    state = oldstate * PCG32_MULT + inc;
    std::uint32_t xorshifted = (std::uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    std::uint32_t rot = (std::uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
  }

  /**
   * @brief Generates a 64-bit unsigned integer.
   *
   * Combines two 32-bit random numbers to produce a 64-bit random number.
   *
   * @return A 64-bit unsigned integer.
   */
  template <> inline std::uint64_t Rng::uniform<std::uint64_t>() {
    std::uint64_t v0 = uniform<std::uint32_t>(), v1 = uniform<std::uint32_t>();
    return (v0 << 32) | v1;
  }

  /**
   * @brief Generates a 32-bit signed integer.
   *
   * Adjusts a 32-bit unsigned random number to fit within the signed integer range. This is done
   * to ensure that no undefined behavior occurs when converting the unsigned value to a signed
   * value.
   *
   * @return A 32-bit signed integer.
   * @see
   * https://stackoverflow.com/questions/13150449/efficient-unsigned-to-signed-cast-avoiding-implementation-defined-behavior/13208789#13208789
   */
  template <> inline std::int32_t Rng::uniform<std::int32_t>() {
    std::uint32_t v = uniform<std::uint32_t>();
    if (v <= (std::uint32_t)std::numeric_limits<std::int32_t>::max())
      return std::int32_t(v);
    DASSERT_GE(v, (std::uint32_t)std::numeric_limits<std::int32_t>::min());
    return std::int32_t(v - std::numeric_limits<std::int32_t>::min()) +
           std::numeric_limits<std::int32_t>::min();
  }

  /**
   * @brief Generates a 64-bit signed integer.
   *
   * Adjusts a 64-bit unsigned random number to fit within the signed integer range. This is done
   * to ensure that no undefined behavior occurs when converting the unsigned value to a signed
   * value.
   *
   * @return A 64-bit signed integer.
   * @see
   * https://stackoverflow.com/questions/13150449/efficient-unsigned-to-signed-cast-avoiding-implementation-defined-behavior/13208789#13208789
   */
  template <> inline std::int64_t Rng::uniform<std::int64_t>() {
    std::uint64_t v = uniform<std::uint64_t>();
    if (v <= (std::uint64_t)std::numeric_limits<std::int64_t>::max())
      return std::int64_t(v);
    DASSERT_GE(v, (std::uint64_t)std::numeric_limits<std::int64_t>::min());
    return std::int64_t(v - std::numeric_limits<std::int64_t>::min()) +
           std::numeric_limits<std::int64_t>::min();
  }

  /**
   * @brief Generates a random float between 0 and 1.
   *
   * Scales a 32-bit unsigned random number to the floating-point range \f$[0, 1)\f$.
   *
   * @return A random float.
   */
  template <> inline float Rng::uniform<float>() {
    return std::min<float>(OneMinusEpsilon, uniform<std::uint32_t>() * 0x1p-32f);
  }

  /**
   * @brief Generates a random double between 0 and 1.
   *
   * Scales a 64-bit unsigned random number to the floating-point range \f$[0, 1)\f$.
   *
   * @return A random double.
   */
  template <> inline double Rng::uniform<double>() {
    return std::min<double>(OneMinusEpsilon, uniform<std::uint64_t>() * 0x1p-64f);
  }

  inline void Rng::set_sequence(std::uint64_t seq_index, std::uint64_t offset) {
    state = 0;
    inc = (seq_index << 1u) | 1u;
    uniform<std::uint32_t>();
    state += offset;
    uniform<std::uint32_t>();
  }

} // namespace specula

#endif // SPECULA_UTIL_RNG_HPP_
