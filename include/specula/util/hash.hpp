#ifndef SPECULA_UTIL_HASH_HPP
#define SPECULA_UTIL_HASH_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "specula/macros.hpp"
#include "specula/types.hpp"

namespace specula {

  SPECULA_CPU_GPU inline uint64_t murmur_hash_64A(const uint8_t *key, size_t len, uint64_t seed) {
    const uint64_t m = 0xc6a4a7935bd1e995ull;
    const int r = 47;

    uint64_t h = seed ^ (len * m);
    const uint8_t *end = key + 8 * (len / 8);

    while (key != end) {
      uint64_t k = 0;
      std::memcpy(&k, key, sizeof(uint64_t));
      key += 8;

      k *= m;
      k ^= k >> r;
      k *= m;

      h ^= k;
      h *= m;
    }

    switch (len & 7) { // NOLINT
    case 7:
      h ^= uint64_t(key[6]) << 48;
    case 6:
      h ^= uint64_t(key[5]) << 40;
    case 5:
      h ^= uint64_t(key[4]) << 32;
    case 4:
      h ^= uint64_t(key[3]) << 24;
    case 3:
      h ^= uint64_t(key[2]) << 16;
    case 2:
      h ^= uint64_t(key[1]) << 8;
    case 1:
      h ^= uint64_t(key[0]);
      h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
  }

  SPECULA_CPU_GPU inline uint64_t mix_bits(uint64_t v) {
    v ^= (v >> 31);
    v *= 0x7fb5d329728ea185;
    v ^= (v >> 27);
    v *= 0x81dadef4bc2dd44d;
    v ^= (v >> 33);
    return v;
  }

  template <typename T>
  SPECULA_CPU_GPU inline uint64_t hash_buffer(const T *ptr, size_t n_elements, uint64_t seed = 0) {
    return murmur_hash_64A((const uint8_t *)ptr, n_elements * sizeof(T), seed);
  }

  template <typename... Args>
  SPECULA_CPU_GPU inline void hash_recursive_copy(uint8_t *buf, Args...);
  template <> SPECULA_CPU_GPU inline void hash_recursive_copy(uint8_t *buf) {}
  template <typename T, typename... Args>
  SPECULA_CPU_GPU inline void hash_recursive_copy(uint8_t *buf, T v, Args... args) {
    std::memcpy(buf, &v, sizeof(T));
    hash_recursive_copy(buf + sizeof(T), args...);
  }

  template <typename... Args> SPECULA_CPU_GPU inline uint64_t hash(Args... args) {
    constexpr size_t size = (sizeof(Args) + ... + 0);
    constexpr size_t n = (size + 7) / 8;
    uint64_t buf[n];
    hash_recursive_copy((uint8_t *)buf, args...);
    return murmur_hash_64A((const uint8_t *)buf, size, 0);
  }

  template <typename... Args> SPECULA_CPU_GPU inline Float hash_float(Args... args) {
    return uint32_t(hash(args...)) * 0x1p-32f;
  }
} // namespace specula

#endif // SPECULA_UTIL_HASH_HPP
