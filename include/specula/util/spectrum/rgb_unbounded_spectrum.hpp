#ifndef INCLUDE_SPECTRUM_RGB_UNBOUNDED_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_RGB_UNBOUNDED_SPECTRUM_HPP_

#include "specula.hpp"
#include "specula/util/color/rgb.hpp"
#include "specula/util/color/rgb_sigmoid_polynomial.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"

namespace specula {
  class RgbUnboundedSpectrum {
  public:
    SPECULA_CPU_GPU RgbUnboundedSpectrum() : rsp(0, 0, 0), scale(0) {}
    SPECULA_CPU_GPU RgbUnboundedSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        s[i] = scale * rsp(lambda[i]);
      }
      return s;
    }

    SPECULA_CPU_GPU Float max_value() const { return scale * rsp.max_value(); }
    SPECULA_CPU_GPU Float operator()(Float lambda) const { return scale * rsp(lambda); }

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

#endif // INCLUDE_SPECTRUM_RGB_UNBOUNDED_SPECTRUM_HPP_
