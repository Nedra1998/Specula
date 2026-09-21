#ifndef SPECULA_UTIL_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP

#include "specula/macros.hpp"
#include "specula/util/color/rgb.hpp"
#include "specula/util/color/rgb_sigmoid_polynomial.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"

namespace specula {
  class RgbAlbedoSpectrum {
  public:
    SPECULA_CPU_GPU RgbAlbedoSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU Float operator()(Float lambda) const { return rsp(lambda); }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return rsp.max_value(); }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = rsp(lambda[i]);
      }
      return s;
    }

  private:
    RgbSigmoidPolynomial rsp;
  };
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP
