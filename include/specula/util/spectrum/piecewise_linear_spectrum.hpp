#ifndef SPECULA_UTIL_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"

namespace specula {
  class Spectrum;

  class PiecewiseLinearSpectrum {
  public:
    static pstd::optional<Spectrum> read(const std::string &filename, Allocator alloc);
    static PiecewiseLinearSpectrum *from_interleaved(pstd::span<const Float> samples,
                                                     bool normalize, Allocator alloc);

    PiecewiseLinearSpectrum() = default;
    PiecewiseLinearSpectrum(pstd::span<const Float> lambda, pstd::span<const Float> values,
                            Allocator alloc = {});

    SPECULA_CPU_GPU void scale(Float s) {
      for (Float &v : values) {
        v *= s;
      }
    }

    SPECULA_CPU_GPU Float operator()(Float lambda) const;

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const;

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;

      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = (*this)(lambda[i]);
      }

      return s;
    }

  private:
    pstd::vector<Float> lambdas, values;
  };
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP
