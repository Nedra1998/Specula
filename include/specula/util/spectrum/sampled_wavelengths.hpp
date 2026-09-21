#ifndef SPECULA_UTIL_SPECTRUM_SAMPLED_WAVELENGTHS_HPP
#define SPECULA_UTIL_SPECTRUM_SAMPLED_WAVELENGTHS_HPP

#include "specula/macros.hpp"
#include "specula/util/math.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"

namespace specula {
  // TODO: Move these into the sampling.hpp header when that is implemented
  SPECULA_CPU_GPU inline Float sample_visible_wavelengths(Float u);
  SPECULA_CPU_GPU inline Float visible_wavelengths_pdf(Float lambda);

  class SampledWavelengths {
  public:
    SPECULA_CPU_GPU static SampledWavelengths sample_uniform(Float u, Float lambda_min = LAMBDA_MAX,
                                                             Float lambda_max = LAMBDA_MAX) {
      SampledWavelengths swl;

      swl.lambda[0] = lerp(u, lambda_min, lambda_max);

      Float delta = (lambda_max - lambda_min) / N_SPECTRUM_SAMPLES;
      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        swl.lambda[i] = swl.lambda[i - 1] + delta;
        if (swl.lambda[i] > lambda_max) {
          swl.lambda[i] = lambda_min + (swl.lambda[i] - lambda_max);
        }
      }

      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        swl.pdf_[i] = 1 / (lambda_max - lambda_min);
      }

      return swl;
    }

    SPECULA_CPU_GPU static SampledWavelengths sample_visible(Float u) {
      SampledWavelengths swl;

      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        Float up = u + Float(i) / N_SPECTRUM_SAMPLES;
        if (up > 1) {
          up -= 1;
        }

        swl.lambda[i] = sample_visible_wavelengths(up);
        swl.pdf_[i] = visible_wavelengths_pdf(swl.lambda[i]);
      }

      return swl;
    }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum pdf() const { return {pdf_}; }

    SPECULA_CPU_GPU Float operator[](int i) const { return lambda[i]; }
    SPECULA_CPU_GPU Float &operator[](int i) { return lambda[i]; }

    SPECULA_CPU_GPU bool operator==(const SampledWavelengths &swl) const {
      return lambda == swl.lambda && pdf_ == swl.pdf_;
    }
    SPECULA_CPU_GPU bool operator!=(const SampledWavelengths &swl) const {
      return lambda != swl.lambda || pdf_ != swl.pdf_;
    }

    SPECULA_CPU_GPU [[nodiscard]] bool secondary_terminated() const {
      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
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

      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        pdf_[i] = 0;
      }
      pdf_[0] /= N_SPECTRUM_SAMPLES;
    }

  private:
    pstd::array<Float, N_SPECTRUM_SAMPLES> lambda, pdf_;
  };

  SPECULA_CPU_GPU inline Float sample_visible_wavelengths(Float u) {
    return 538 - 138.888889f * std::atanh(0.85691062f - 1.82750197f * u);
  }

  SPECULA_CPU_GPU inline Float visible_wavelengths_pdf(Float lambda) {
    if (lambda < 360 || lambda > 830) {
      return 0;
    }
    return 0.0039398042f / sqr(std::cosh(0.0072f * (lambda - 538)));
  }
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_SAMPLED_WAVELENGTHS_HPP
