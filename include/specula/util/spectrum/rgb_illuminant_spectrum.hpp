#ifndef INCLUDE_SPECTRUM_RGB_ILLUMINANT_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_RGB_ILLUMINANT_SPECTRUM_HPP_

#include "specula.hpp"
#include "specula/util/color/rgb.hpp"
#include "specula/util/color/rgb_sigmoid_polynomial.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/densly_sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"

namespace specula {
  class RgbIlluminantSpectrum {
  public:
    RgbIlluminantSpectrum() = default;
    SPECULA_CPU_GPU RgbIlluminantSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU const DenslySampledSpectrum *illuminant() const { return illuminant_; }

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const {
      if (!illuminant_) {
        return SampledSpectrum(0);
      }
      SampledSpectrum s;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        s[i] = scale * rsp(lambda[i]);
      }
      return s * illuminant_->sample(lambda);
    }

    SPECULA_CPU_GPU Float max_value() const {
      if (!illuminant_) {
        return 0;
      }
      return scale * rsp.max_value() * illuminant_->max_value();
    }

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      if (!illuminant_) {
        return 0;
      }
      return scale * rsp(lambda) * (*illuminant_)(lambda);
    }

  private:
    Float scale;
    RgbSigmoidPolynomial rsp;
    const DenslySampledSpectrum *illuminant_;

    friend struct fmt::formatter<RgbIlluminantSpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::RgbIlluminantSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RgbIlluminantSpectrum &v, FormatContext &ctx) const {
    if (v.illuminant_) {
      return format_to(ctx.out(), "[ RgbIlluminantSpectrum rsp={} scale={} illuminant={} ]", v.rsp,
                       v.scale, (*v.illuminant_));
    } else {
      return format_to(ctx.out(), "[ RgbIlluminantSpectrum rsp={} scale={} illuminant=(nullptr) ]",
                       v.rsp, v.scale);
    }
  }
};

#endif // INCLUDE_SPECTRUM_RGB_ILLUMINANT_SPECTRUM_HPP_
