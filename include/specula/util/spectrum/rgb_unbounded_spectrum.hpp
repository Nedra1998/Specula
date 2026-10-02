#ifndef SPECULA_UTIL_SPECTRUM_RGB_UNBOUNDED_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_RGB_UNBOUNDED_SPECTRUM_HPP

// IWYU pragma: private, include "specula/util/spectrum.hpp"

#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/spectrum/spectrum.hpp"

namespace specula {
  class RgbUnboundedSpectrum {
  public:
    SPECULA_CPU_GPU RgbUnboundedSpectrum() : rsp(0, 0, 0), scale(0) {}
    SPECULA_CPU_GPU RgbUnboundedSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU Float operator()(Float lambda) const { return scale * rsp(lambda); }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return scale * rsp.max_value(); }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = scale * rsp(lambda[i]);
      }
      return s;
    }

  private:
    Float scale = 1;
    RgbSigmoidPolynomial rsp;

    friend struct fmt::formatter<RgbUnboundedSpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::RgbUnboundedSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RgbUnboundedSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ RgbUnboundedSpectrum rsp={} ]", v.rsp);
  }
};

#endif // SPECULA_UTIL_SPECTRUM_RGB_UNBOUNDED_SPECTRUM_HPP
