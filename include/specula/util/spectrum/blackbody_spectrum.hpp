#ifndef SPECULA_UTIL_SPECTRUM_BLACKBODY_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_BLACKBODY_SPECTRUM_HPP

// IWYU pragma: private, include "specula/util/spectrum.hpp"

#include "specula/util/spectrum/functions.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/spectrum/spectrum.hpp"

namespace specula {
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

    friend struct fmt::formatter<BlackbodySpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::BlackbodySpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::BlackbodySpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ BlackbodySpectrum T={} ]", v.T);
  }
};

#endif // SPECULA_UTIL_SPECTRUM_BLACKBODY_SPECTRUM_HPP
