#ifndef INCLUDE_UTIL_HASH_HPP_
#define INCLUDE_UTIL_HASH_HPP_

#include "specula/specula.hpp"

namespace specula {
  /**
   * @brief Mixes the bits of a 64-bit integer as a hash finalizer
   *
   * Hashing functions make use a a finalizer to take an intermediate hash value and increase its
   * entropy. The goal should be that flipping a single bit in the input key results in all output
   * bits changing with a probability of 50%. This is called the [avalanche
   * effect](https://en.wikipedia.org/wiki/Avalanche_effect).
   *
   * This function is derived from the MurmurHash3 64-bit finalizer, with some modifications to
   * improve the quality of the finalizer specifically for low-entropy input data (such as counting
   * numbers).
   *
   * @param v The 64-bit integer to mix
   * @return A 64-bit integer with the bits mixed
   *
   * @see http://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html
   */
  SPECULA_CPU_GPU inline std::uint64_t mix_bits(std::uint64_t v) {
    v ^= (v >> 31);
    v *= 0x7fb5d329728ea185;
    v ^= (v >> 27);
    v *= 0x81dadef4bc2dd44d;
    v ^= (v >> 33);
    return v;
  }

  /**
   * @brief MurmurHash64A is a 64-bit hash function that is optimized for x64 architecture
   *
   * It operates by processing data in 8-byte chunks, mixing each chunk into the hash state through
   * bitwise operations, shifts, and multiplications with large constants. These operations ensure
   * that even small changes in the input result in vastly different hash outputs, providing a good
   * distribution of hash values. Despite its speed and simplicity, MurmurHash64A is not suitable
   * for cryptographic purposes, but it excels in scenarios like hash tables and checksumming where
   * speed and uniform distribution are essential. The algorithm is designed for 64-bit
   * architectures and provides a 64-bit hash value.
   *
   * @param key The input data to hash
   * @param len The length of the input data
   * @param seed Initial seed value for the hash
   * @return A 64-bit hash of the input data
   * @see https://en.wikipedia.org/wiki/MurmurHash
   * @see https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.cpp
   */
  SPECULA_CPU_GPU inline std::uint64_t murmur_hash_64A(const std::uint8_t *key, std::size_t len,
                                                       std::uint64_t seed) {
    const uint64_t m = 0xc6a4a7935bd1e995ull;
    const int r = 47;

    std::uint64_t h = seed ^ (len * m);
    const std::uint8_t *end = key + 8 * (len / 8);

    while (key != end) {
      std::uint64_t k;
      std::memcpy(&k, key, sizeof(std::uint64_t));
      key += 8;

      k *= m;
      k ^= k >> r;
      k *= m;

      h ^= k;
      h *= m;
    }

    switch (len & 7) {
    case 7:
      h ^= std::uint64_t(key[6]) << 48;
    case 6:
      h ^= std::uint64_t(key[5]) << 40;
    case 5:
      h ^= std::uint64_t(key[4]) << 32;
    case 4:
      h ^= std::uint64_t(key[3]) << 24;
    case 3:
      h ^= std::uint64_t(key[2]) << 16;
    case 2:
      h ^= std::uint64_t(key[1]) << 8;
    case 1:
      h ^= std::uint64_t(key[0]);
      h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
  }

  /**
   * @brief Hash a buffer of data
   *
   * This directly utilizes the `murmur_hash_64A` function to hash a buffer of data.
   *
   * @tparam T The type of the data to hash
   * @param ptr A pointer to the input data
   * @param size The size in bytes of the input data
   * @param seed An initial seed value for the hash
   * @return A 64-bit hash of the input data
   */
  template <typename T>
  SPECULA_CPU_GPU inline std::uint64_t hash_buffer(const T *ptr, size_t size,
                                                   std::uint64_t seed = 0) {
    return murmur_hash_64A(reinterpret_cast<const std::uint8_t *>(ptr), size, seed);
  }

  /**
   * @brief Copy a sequence of values into a byte buffer
   *
   * This method should not be used directly, and is used in conjunction with `hash` to copy a
   * sequence of values into a byte buffer. The buffer is then hashed using a hash function to
   * produce a single hash value.
   *
   * @tparam Args The types of the values to copy
   * @param buf The destination buffer to copy the bytes into
   */
  template <typename... Args>
  SPECULA_CPU_GPU inline void hash_recursive_copy(std::uint8_t *buf, Args...);

  template <> SPECULA_CPU_GPU inline void hash_recursive_copy(std::uint8_t *buf) {}

  template <typename T, typename... Args>
  SPECULA_CPU_GPU inline void hash_recursive_copy(std::uint8_t *buf, T v, Args... args) {
    std::memcpy(buf, &v, sizeof(T));
    hash_recursive_copy(buf + sizeof(T), args...);
  }

  /**
   * @brief Hash an arbitrary sequence of values
   *
   * This function takes an arbitrary number of arguments, copies them into a byte buffer (using
   * `hash_recursive_copy`), and then hashes the buffer using a hash function (`murmur_hash_64A`) to
   * produce a single hash value.
   *
   * @tparam Args The types of the values to hash
   * @param args The values to hash
   * @return The hash of the combined input values
   * @see https://en.cppreference.com/w/cpp/language/fold
   */
  template <typename... Args> SPECULA_CPU_GPU inline std::uint64_t hash(Args... args) {
    constexpr std::size_t size = (sizeof(Args) + ... + 0);
    constexpr std::size_t n = (size + 7) / 8;
    std::uint64_t buf[n];
    hash_recursive_copy((std::uint8_t *)buf, args...);
    return murmur_hash_64A((const std::uint8_t *)buf, size, 0);
  }

  /**
   * @brief Generate a hash value represented as a floating-point number
   *
   * Sometimes it is useful to have a floating-point representation of a hash value. This function
   * hanles the conversion from the `hash` method and converts it into a floating point value
   * between \f(0\f) and \f(1\f).
   *
   * @tparam Args The types of the values to be hashed
   * @param args The values to hash
   * @return A floating-point representation of the hash value
   * @see hash
   */
  template <typename... Args> SPECULA_CPU_GPU inline Float hash_float(Args... args) {
    return std::uint32_t(hash(args...)) * 0x1p-32f;
  }
} // namespace specula

#endif // INCLUDE_UTIL_HASH_HPP_
