/**
 * @file specula.hpp
 * @brief Main entrypoint for the renderer
 *
 * This file provides the main entrypoint for the renderer library, and includes some global
 * initialization and teardown logic that should be called first and last thing respectivly.
 */
#ifndef SPECULA_HPP
#define SPECULA_HPP

#include <cstdint>
#include <vector>

#include <spdlog/spdlog.h>

#if defined(__CUDA_ARCH__)
#  define SPECULA_IS_GPU_CODE
#endif

#if defined(SPECULA_BUILD_GPU) && defined(__CUDACC__)
#  ifndef SPECULA_NOINLINE
#    define SPECULA_NOINLINE __attribute((noinline))
#  endif
#  define SPECULA_CPU_GPU __host__ __device__
#  define SPECULA_GPU __device
#  if defined(SPECULA_IS_GPU_CODE)
#    define SPECULA_CONST __device__ const
#  else
#    define SPECULA_CONST const
#  endif
#else
#  define SPECULA_CONST const
#  define SPECULA_CPU_GPU
#  define SPECULA_GPU
#endif

/**
 * @brief Primary namespace
 *
 * This is the primary namespace for the specula renderer. All components of the renderer will be
 * contained within this namespace, to avoid any naming conflicts with external packages.
 */
namespace specula {

#ifdef SPECULA_FLOAT_AS_DOUBLE
  /// @brief Type alias for floating point numbers
  using Float = double;
  /// @brief Type alias for the bit representation of floating point numbers
  using FloatBits = uint64_t;
#else
  /// @brief Type alias for floating point numbers
  using Float = float;
  /// @brief Type alias for the bit representation of floating point numbers
  using FloatBits = uint32_t;
#endif

  static_assert(sizeof(Float) == sizeof(FloatBits), "Float and FloatBits must have the same size");

  /**
   * @brief Initializes the specula renderer
   *
   * @param log_sinks An optional list of sinks to attach to the default logger
   * @return true if all initialization was successful
   * @return false if there were any errors during initialization
   * @see logging::initialize
   */
  bool initialize(std::vector<spdlog::sink_ptr> log_sinks = {});
} // namespace specula

#endif // SPECULA_HPP
