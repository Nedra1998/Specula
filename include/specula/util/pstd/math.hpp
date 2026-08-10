#ifndef SPECULA_UTIL_PSTD_MATH_HPP
#define SPECULA_UTIL_PSTD_MATH_HPP

#include <cmath>

#include "specula/macros.hpp"

namespace specula::pstd {
  SPECULA_CPU_GPU inline float sqrt(float f) { return ::sqrtf(f); }
  SPECULA_CPU_GPU inline double sqrt(double f) { return ::sqrt(f); }
  SPECULA_CPU_GPU inline float abs(float f) { return ::fabsf(f); }
  SPECULA_CPU_GPU inline double abs(double f) { return ::fabs(f); }

  SPECULA_CPU_GPU inline float copysign(float mag, float sign) {
#ifdef SPECULA_IS_GPU_CODE
    return ::copysignf(mag, sign);
#else
    return std::copysign(mag, sign);
#endif
  }
  SPECULA_CPU_GPU inline double copysign(double mag, double sign) {
#ifdef SPECULA_IS_GPU_CODE
    return ::copysign(mag, sign);
#else
    return std::copysign(mag, sign);
#endif
  }

  SPECULA_CPU_GPU inline float floor(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::floorf(arg);
#else
    return std::floor(arg);
#endif
  }
  SPECULA_CPU_GPU inline double floor(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::floor(arg);
#else
    return std::floor(arg);
#endif
  }

  SPECULA_CPU_GPU inline float ceil(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::ceilf(arg);
#else
    return std::ceil(arg);
#endif
  }
  SPECULA_CPU_GPU inline double ceil(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::ceil(arg);
#else
    return std::ceil(arg);
#endif
  }

  SPECULA_CPU_GPU inline float round(float arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::roundf(arg);
#else
    return std::round(arg);
#endif
  }
  SPECULA_CPU_GPU inline double round(double arg) {
#ifdef SPECULA_IS_GPU_CODE
    return ::round(arg);
#else
    return std::round(arg);
#endif
  }

  SPECULA_CPU_GPU inline float fmod(float x, float y) {
#ifdef SPECULA_IS_GPU_CODE
    return ::fmodf(x, y);
#else
    return std::fmod(x, y);
#endif
  }
  SPECULA_CPU_GPU inline double fmod(double x, double y) {
#ifdef SPECULA_IS_GPU_CODE
    return ::fmod(x, y);
#else
    return std::fmod(x, y);
#endif
  }
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_MATH_HPP
