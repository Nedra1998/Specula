#ifndef INCLUDE_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP_

#include "specula.hpp"
#include "specula/util/color/rgb.hpp"
#include "specula/util/color/rgb_sigmoid_polynomial.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"

namespace specula {
  class RgbAlbedoSpectrum {
  public:
    SPECULA_CPU_GPU RgbAlbedoSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        s[i] = rsp(lambda[i]);
      }
      return s;
    }

    SPECULA_CPU_GPU Float max_value() const { return rsp.max_value(); }
    SPECULA_CPU_GPU Float operator()(Float lambda) const { return rsp(lambda); }

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

#endif // INCLUDE_SPECTRUM_RGB_ALBEDO_SPECTRUM_HPP_
