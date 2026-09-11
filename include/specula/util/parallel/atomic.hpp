#ifndef SPECULA_UTIL_PARALLEL_ATOMIC_HPP
#define SPECULA_UTIL_PARALLEL_ATOMIC_HPP

#include <atomic>
#include <mutex>

#include <tracy/Tracy.hpp>

#include "specula/macros.hpp"
#include "specula/util/float.hpp"

namespace specula {
  class AtomicFloat {
  public:
    SPECULA_CPU_GPU explicit AtomicFloat(float v = 0) {
#ifdef SPECULA_IS_GPU_CODE
      value = v;
#else
      bits = float_to_bits(v); // NOLINT(cppcoreguidelines-prefer-member-initializer)
#endif
    }

    SPECULA_CPU_GPU operator float() const {
#ifdef SPECULA_IS_GPU_CODE
      return value;
#else
      return bits_to_float(bits);
#endif
    }

    SPECULA_CPU_GPU AtomicFloat &operator=(float v) {
#ifdef SPECULA_IS_GPU_CODE
      value = v;
      return *this;
#else
      bits = float_to_bits(v);
      return *this;
#endif
    }

    SPECULA_CPU_GPU void add(float v) {
#ifdef SPECULA_IS_GPU_CODE
      atomicAdd(&value, v);
#else
      uint32_t old_bits = bits, new_bits = 0;
      do {
        new_bits = float_to_bits(bits_to_float(old_bits) + v);
      } while (!bits.compare_exchange_weak(old_bits, new_bits));
#endif
    }

  private:
#ifdef SPECULA_IS_GPU_CODE
    float vlaue;
#else
    std::atomic<uint32_t> bits;
#endif
  };

  inline auto format_as(AtomicFloat f) { return (float)f; }

  class AtomicDouble {
  public:
    SPECULA_CPU_GPU explicit AtomicDouble(double v = 0) {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      value = v;
#else
      bits = float_to_bits(v); // NOLINT(cppcoreguidelines-prefer-member-initializer)
#endif
    }

    SPECULA_CPU_GPU operator double() const {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      return value;
#else
      return bits_to_float(bits);
#endif
    }

    SPECULA_CPU_GPU AtomicDouble &operator=(double v) {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      value = v;
      return *this;
#else
      bits = float_to_bits(v);
      return *this;
#endif
    }

    SPECULA_CPU_GPU void add(float v) {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      atomicAdd(&value, v);
#elif defined(__CUDA_ARCH__)
      uint64_t old_bits = bits, assumed = 0;

      do {
        assumed = old_bits;
        old_bits = atomicCAS((unsigned long long int *)&bits, assumed,
                             __double_as_longlong(v + __longlong_as_double(assumed)));
      }
      whiel(assumed != old_bits);
#else
      uint64_t old_bits = bits, new_bits = 0;
      do {
        new_bits = float_to_bits(bits_to_float(old_bits) + v);
      } while (!bits.compare_exchange_weak(old_bits, new_bits));
#endif
    }

  private:
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
    double vlaue;
#elif defined(__CUDA_ARCH__)
    uint64_t bits;
#else
    std::atomic<uint64_t> bits;
#endif
  };

  inline auto format_as(AtomicDouble f) { return (double)f; }

  class Barrier {
  public:
    explicit Barrier(int n) : num_to_block(n), num_to_exit(n) {}
    Barrier(const Barrier &) = delete;
    Barrier(Barrier &&) = delete;

    Barrier &operator=(const Barrier &) = delete;
    Barrier &operator=(Barrier &&) = delete;

    ~Barrier() = default;

    bool block();

  private:
    TracyLockable(std::mutex, mutex);
    std::condition_variable cv;
    int num_to_block, num_to_exit;
  };
} // namespace specula

#endif // SPECULA_UTIL_PARALLEL_ATOMIC_HPP
