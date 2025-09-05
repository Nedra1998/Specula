#ifndef INCLUDE_SPECTRUM_BLACKBODY_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_BLACKBODY_SPECTRUM_HPP_

#include "specula.hpp"
#include "util/spectrum/constants.hpp"
#include "util/spectrum/functions.hpp"
#include "util/spectrum/sampled_spectrum.hpp"
#include "util/spectrum/sampled_wavelengths.hpp"

namespace specula {

  class BlackbodySpectrum {
  public:
    SPECULA_CPU_GPU BlackbodySpectrum(Float t) : t(t) {
      Float lambda_max = 2.8977721e-3f / t;
      normalization_factor = 1 / blackbody(lambda_max * 1e9f, t);
    }

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        s[i] = blackbody(lambda[i], t) * normalization_factor;
      }
      return s;
    }
    SPECULA_CPU_GPU Float max_value() const { return 1.0f; }

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      return blackbody(lambda, t) * normalization_factor;
    }

  private:
    Float t;
    Float normalization_factor;
  };
} // namespace specula

#endif // INCLUDE_SPECTRUM_BLACKBODY_SPECTRUM_HPP_
