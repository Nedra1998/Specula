#ifndef INCLUDE_PARALLEL_ATOMIC_HPP_
#define INCLUDE_PARALLEL_ATOMIC_HPP_

#include "specula.hpp"
#include "util/float.hpp"

namespace specula {
  class AtomicFloat {
  public:
    SPECULA_CPU_GPU explicit AtomicFloat(float v = 0) {
#ifdef SPECULA_IS_GPU_CODE
      value = v;
#else
      bits = float_to_bits(v);
#endif
    }

    SPECULA_CPU_GPU operator float() const {
#ifdef SPECULA_IS_GPU_CODE
      return value;
#else
      return bits_to_float(bits);
#endif
    }

    SPECULA_CPU_GPU float operator=(float v) {
#ifdef SPECULA_IS_GPU_CODE
      value = v;
      return value;
#else
      bits = float_to_bits(v);
      return v;
#endif
    }

    SPECULA_CPU_GPU
    void add(float v) {
#ifdef SPECULA_IS_GPU_CODE
      atomicAdd(&value, v);
#else
      uint32_t old_bits = bits, new_bits;
      do {
        new_bits = float_to_bits(bits_to_float(old_bits) + v);
      } while (!bits.compare_exchange_weak(old_bits, new_bits));
#endif
    }

  private:
#ifdef SPECULA_IS_GPU_CODE
    float value;
#else
    std::atomic<uint32_t> bits;
#endif
  };

  class AtomicDouble {
  public:
    SPECULA_CPU_GPU explicit AtomicDouble(double v = 0) {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      value = v;
#else
      bits = float_to_bits(v);
#endif
    }

    SPECULA_CPU_GPU operator double() const {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      return value;
#else
      return bits_to_float(bits);
#endif
    }

    SPECULA_CPU_GPU double operator=(double v) {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      value = v;
      return value;
#else
      bits = float_to_bits(v);
      return v;
#endif
    }

    SPECULA_CPU_GPU void add(double v) {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ >= 600)
      atomicAdd(&value, v);
#elif defined(__CUDA_ARCH__)
      uint64_t old = bits, assumed;
      do {
        assumed = old;
        old = atomicCAS((unsigned long long int *)&bits, assumed,
                        __double_as_longlong(v + __longlong_as_double(assumed)));
      } while (assumed != old);
#else
      uint64_t old_bits = bits, new_bits;
      do {
        new_bits = float_to_bits(bits_to_float(old_bits) + v);
      } while (!bits.compare_exchange_weak(old_bits, new_bits));
#endif
    }

  private:
#if (defined(__CUDE_ARCH__) && __CUDA_ARCH__ >= 600)
    double value;
#elif defined(__CUDA_ARCH__)
    uint64_t bits;
#else
    std::atomic<uint64_t> bits;
#endif
  };

  class Barrier {
  public:
    explicit Barrier(int n) : num_to_block_(n), num_to_exit_(n) {}
    Barrier(const Barrier &) = delete;
    Barrier &operator=(const Barrier &) = delete;

    bool block();

  private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int num_to_block_, num_to_exit_;
  };

  inline auto format_as(AtomicFloat f) { return (float)f; }
  inline auto format_as(AtomicDouble f) { return (double)f; }
} // namespace specula

#endif // INCLUDE_PARALLEL_ATOMIC_HPP_
