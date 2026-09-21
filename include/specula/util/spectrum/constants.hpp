#ifndef SPECULA_UTIL_SPECTRUM_CONSTANTS_HPP
#define SPECULA_UTIL_SPECTRUM_CONSTANTS_HPP

#include "specula/types.hpp"

namespace specula {
  static constexpr Float LAMBDA_MIN = 360, LAMBDA_MAX = 830;
  static constexpr int N_SPECTRUM_SAMPLES = 4;
  static constexpr Float CIE_Y_INTEGRAL = 106.856895;
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_CONSTANTS_HPP
