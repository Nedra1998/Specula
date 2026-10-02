#ifndef SPECULA_UTIL_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP

// IWYU pragma: private, include "specula/util/spectrum.hpp"

#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/spectrum/spectrum.hpp"

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

    friend struct fmt::formatter<RgbAlbedoSpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::RgbAlbedoSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RgbAlbedoSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ RgbAlbedoSpectrum rsp={} ]", v.rsp);
  }
};

#endif // SPECULA_UTIL_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP
