#ifndef INCLUDE_SPECTRUM_SAMPLED_WAVELENGTHS_HPP_
#define INCLUDE_SPECTRUM_SAMPLED_WAVELENGTHS_HPP_

#include "util/math/functions.hpp"
#include "util/pstd/array.hpp"
#include "util/spectrum/constants.hpp"
#include "util/spectrum/sampled_spectrum.hpp"

namespace specula {
  class SampledWavelengths {
  public:
    SampledWavelengths() = default;

    SPECULA_CPU_GPU static SampledWavelengths sample_uniform(Float u, Float lambda_min = LAMBDA_MIN,
                                                             Float lambda_max = LAMBDA_MAX) {
      SampledWavelengths swl;

      swl.lambda[0] = lerp(u, lambda_min, lambda_max);

      Float delta = (lambda_max - lambda_min) / NSpectrumSamples;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        swl.lambda[i] = swl.lambda[i - 1] + delta;
        if (swl.lambda[i] > lambda_max) {
          swl.lambda[i] = lambda_min + (swl.lambda[i] - lambda_max);
        }
      }

      for (int i = 0; i < NSpectrumSamples; ++i) {
        swl.pdf_[i] = 1 / (lambda_max - lambda_min);
      }

      return swl;
    }

    SPECULA_CPU_GPU static SampledWavelengths sample_visible(Float u) {
      SampledWavelengths swl;

      for (int i = 0; i < NSpectrumSamples; ++i) {
        Float up = u + Float(i) / NSpectrumSamples;
        if (up > 1) {
          up -= 1;
        }

        // TODO: Implement once I have implemented the sampling.h header
        // swl.lambda[i] = sample_visible_wavelengths(up);
        // swl.pdf_[i] = visible_wavelengths_pdf(swl.lambda[i]);
      }

      return swl;
    }

    SPECULA_CPU_GPU SampledSpectrum pdf() const { return SampledSpectrum(pdf_); }

    SPECULA_CPU_GPU bool secondary_terminated() const {
      for (int i = 1; i < NSpectrumSamples; ++i) {
        if (pdf_[i] != 0) {
          return false;
        }
      }
      return true;
    }

    SPECULA_CPU_GPU void terminate_secondary() {
      if (secondary_terminated()) {
        return;
      }

      for (int i = 1; i < NSpectrumSamples; ++i) {
        pdf_[i] = 0;
      }
      pdf_[0] /= NSpectrumSamples;
    }

    SPECULA_CPU_GPU Float operator[](int i) const { return lambda[i]; }
    SPECULA_CPU_GPU Float &operator[](int i) { return lambda[i]; }

    SPECULA_CPU_GPU bool operator==(const SampledWavelengths &swl) const {
      return lambda == swl.lambda && pdf_ == swl.pdf_;
    }
    SPECULA_CPU_GPU bool operator!=(const SampledWavelengths &swl) const {
      return lambda != swl.lambda || pdf_ != swl.pdf_;
    }

  private:
    // friend struct Soa<SampledWavelengths>;
    pstd::array<Float, NSpectrumSamples> lambda, pdf_;

    friend struct fmt::formatter<SampledWavelengths>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::SampledWavelengths> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::SampledWavelengths &v, FormatContext &ctx) const {
    // TODO: Implement proper formatting
    return format_to(ctx.out(), "[ {} ]", v.lambda);
  }
};

#endif // INCLUDE_SPECTRUM_SAMPLED_WAVELENGTHS_HPP_
