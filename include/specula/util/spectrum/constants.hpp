#ifndef INCLUDE_SPECTRUM_CONSTANTS_HPP_
#define INCLUDE_SPECTRUM_CONSTANTS_HPP_

#include "specula.hpp"

namespace specula {
  static constexpr Float LAMBDA_MIN = 360, LAMBDA_MAX = 830;
  static constexpr int NSpectrumSamples = 4;
  static constexpr Float CIE_Y_INTEGRAL = 106.856895;
} // namespace specula

#endif // INCLUDE_SPECTRUM_CONSTANTS_HPP_
