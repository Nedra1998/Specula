#ifndef INCLUDE_SPECTRUM_CONSTANT_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_CONSTANT_SPECTRUM_HPP_

#include "specula.hpp"
#include "util/spectrum/sampled_spectrum.hpp"

namespace specula {
  class ConstantSpectrum {
  public:
    SPECULA_CPU_GPU ConstantSpectrum(Float c) : c(c) {}

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &) const {
      return SampledSpectrum(c);
    }

    SPECULA_CPU_GPU Float max_value() const { return c; }

    SPECULA_CPU_GPU Float operator()(Float lambda) const { return c; }

  private:
    Float c;
  };
} // namespace specula

#endif // INCLUDE_SPECTRUM_CONSTANT_SPECTRUM_HPP_
