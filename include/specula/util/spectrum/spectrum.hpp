#ifndef SPECULA_UTIL_SPECTRUM_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_SPECTRUM_HPP

// IWYU pragma: private, include "specula/util/spectrum.hpp"

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/taggedptr.hpp"

namespace specula {
  static constexpr Float LAMBDA_MIN = 360, LAMBDA_MAX = 830;
  static constexpr int N_SPECTRUM_SAMPLES = 4;
  static constexpr Float CIE_Y_INTEGRAL = 106.856895;

  class RgbColorSpace;
  class BlackbodySpectrum;
  class ConstantSpectrum;
  class PiecewiseLinearSpectrum;
  class DenselySampledSpectrum;
  class RgbAlbedoSpectrum;
  class RgbUnboundedSpectrum;
  class RgbIlluminantSpectrum;

  class SampledSpectrum;
  class SampledWavelengths;

  class Spectrum
      : public TaggedPointer<ConstantSpectrum, DenselySampledSpectrum, PiecewiseLinearSpectrum,
                             RgbAlbedoSpectrum, RgbUnboundedSpectrum, RgbIlluminantSpectrum,
                             BlackbodySpectrum> {
  public:
    using TaggedPointer::TaggedPointer;

    SPECULA_CPU_GPU Float operator()(Float lambda) const;
    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const;
    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const;
  };

  SPECULA_CPU_GPU inline Float inner_product(Spectrum f, Spectrum g) {
    Float integral = 0;
    for (Float lambda = LAMBDA_MIN; lambda <= LAMBDA_MAX; ++lambda) {
      integral += f(lambda) * g(lambda);
    }
    return integral;
  }
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_SPECTRUM_HPP
