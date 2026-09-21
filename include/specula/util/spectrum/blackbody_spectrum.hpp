#ifndef SPECULA_UTIL_SPECTRUM_BLACKBODY_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_BLACKBODY_SPECTRUM_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"

namespace specula {
  SPECULA_CPU_GPU inline Float blackbody(Float lambda, Float t) {
    if (t <= 0) {
      return 0;
    }

    const Float c = 299792458.f;
    const Float h = 6.62606957e-34f;
    const Float kb = 1.3806488e-23f;

    Float l = lambda * 1e-9f;
    Float le = (2 * h * c * c) / (pow<5>(l) * (fast_exp((h * c) / (l * kb * t)) - 1));
    ASSERT(!isnan(le));
    return le;
  }

  class BlackbodySpectrum {
  public:
    SPECULA_CPU_GPU BlackbodySpectrum(Float T) : T(T) {
      Float lambda_max = 2.8977721e-3f / T;
      normalization_factor = 1 / blackbody(lambda_max * 1e9f, T);
    }

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      return blackbody(lambda, T) * normalization_factor;
    }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return 1.0f; }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = blackbody(lambda[i], T) * normalization_factor;
      }
      return s;
    }

  private:
    Float T;
    Float normalization_factor;
  };

} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_BLACKBODY_SPECTRUM_HPP
