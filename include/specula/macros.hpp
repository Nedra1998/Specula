#ifndef SPECULA_MACROS_HPP
#define SPECULA_MACROS_HPP

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

#ifdef SPECULA_BUILD_GPU
#  define SPECULA_L1_CACHE_LINE_SIZE 128
#else
#  define SPECULA_L1_CACHE_LINE_SIZE 64
#endif

#endif // SPECULA_MACROS_HPP
