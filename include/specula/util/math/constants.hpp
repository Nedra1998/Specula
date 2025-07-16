#ifndef INCLUDE_MATH_CONSTANTS_HPP_
#define INCLUDE_MATH_CONSTANTS_HPP_

#include "specula.hpp"

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

  constexpr Float ShadowEpsilon = 0.0001f;

  constexpr Float Pi = 3.14159265358979323846;
  constexpr Float InvPi = 0.31830988618379067154;
  constexpr Float Inv2Pi = 0.15915494309189533577;
  constexpr Float Inv4Pi = 0.07957747154594766788;
  constexpr Float PiOver2 = 1.57079632679489661923;
  constexpr Float PiOver4 = 0.78539816339744830961;
  constexpr Float Sqrt2 = 1.41421356237309504880;

#endif // SPECULA_IS_GPU_CODE

} // namespace specula

#endif // INCLUDE_MATH_CONSTANTS_HPP_
