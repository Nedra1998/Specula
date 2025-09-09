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

    friend struct fmt::formatter<BlackbodySpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::BlackbodySpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::BlackbodySpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ BlackbodySpectrum T={} ]", v.t);
  }
};

#endif // INCLUDE_SPECTRUM_BLACKBODY_SPECTRUM_HPP_
