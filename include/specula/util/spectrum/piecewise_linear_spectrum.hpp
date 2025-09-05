#ifndef INCLUDE_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP_

#include "specula.hpp"
#include "util/pstd/vector.hpp"
#include "util/spectrum/constants.hpp"
#include "util/spectrum/sampled_spectrum.hpp"
#include "util/spectrum/sampled_wavelengths.hpp"
#include "util/spectrum/spectrum.hpp"

namespace specula {

  class PiecewiseLinearSpectrum {
  public:
    PiecewiseLinearSpectrum() = default;
    PiecewiseLinearSpectrum(pstd::span<const Float> lambdas, pstd::span<const Float> values,
                            Allocator alloc = {});

    static pstd::optional<Spectrum> read(const std::string &filename, Allocator alloc);
    static PiecewiseLinearSpectrum *from_interleaved(pstd::span<const Float> samples,
                                                     bool normalize, Allocator alloc);

    SPECULA_CPU_GPU void scale(Float s) {
      for (Float &v : values) {
        v *= s;
      }
    }

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        s[i] = (*this)(lambda[i]);
      }
      return s;
    }

    SPECULA_CPU_GPU Float max_value() const;

    SPECULA_CPU_GPU Float operator()(Float lambda) const;

  private:
    pstd::vector<Float> lambdas, values;
  };
} // namespace specula

#endif // INCLUDE_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP_
