#ifndef SPECULA_UTIL_MATH_CONSTANTS_HPP
#define SPECULA_UTIL_MATH_CONSTANTS_HPP

#include <numbers>

#include "specula/types.hpp"
namespace specula {

#ifdef SPECULA_IS_GPU_CODE

#  define ShadowEpsilon 0.0001f
#  define Pi Float(3.14159265358979323846)
#  define InvPi Float(0.31830988618379067154)
#  define Inv2Pi Float(0.15915494309189533577)
#  define Inv4Pi Float(0.07957747154594766788)
#  define PiOver2 Float(1.57079632679489661923)
#  define PiOver4 Float(0.78539816339744830961)
#  define Sqrt2 Float(1.41421356237309504880)

#else

  constexpr Float SHADOW_EPISLON = 0.0001f;

  constexpr Float PI = std::numbers::pi;
  constexpr Float INV_PI = std::numbers::inv_pi;
  constexpr Float INV_2PI = 0.15915494309189533577;
  constexpr Float INV_4PI = 0.07957747154594766788;
  constexpr Float PI_OVER_2 = 1.57079632679489661923;
  constexpr Float PI_OVER_4 = 0.78539816339744830961;
  constexpr Float SQRT2 = std::numbers::sqrt2;

#endif // SPECULA_IS_GPU_CODE

} // namespace specula

#endif // SPECULA_UTIL_MATH_CONSTANTS_HPP
