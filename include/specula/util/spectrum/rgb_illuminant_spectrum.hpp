#ifndef SPECULA_UTIL_SPECTRUM_RGB_ILLUMINANT_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_RGB_ILLUMINANT_SPECTRUM_HPP

#include "specula/macros.hpp"
#include "specula/util/color/rgb.hpp"
#include "specula/util/color/rgb_sigmoid_polynomial.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/densely_sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"

namespace specula {
  class RgbIlluminantSpectrum {
  public:
    SPECULA_CPU_GPU RgbIlluminantSpectrum() = default;
    SPECULA_CPU_GPU RgbIlluminantSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      if (illuminant == nullptr) {
        return 0.0f;
      }
      return scale * rsp(lambda) * (*illuminant)(lambda);
    }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const {
      if (illuminant == nullptr) {
        return 0.0f;
      }
      return scale * rsp.max_value() * illuminant->max_value();
    }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      if (illuminant == nullptr) {
        return SampledSpectrum(0);
      }

      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = scale * rsp(lambda[i]);
      }
      return s * illuminant->sample(lambda);
    }

    const DenselySampledSpectrum *illuminant;

  private:
    Float scale;
    RgbSigmoidPolynomial rsp;
  };
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_RGB_ILLUMINANT_SPECTRUM_HPP
