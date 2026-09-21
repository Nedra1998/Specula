#ifndef SPECULA_UTIL_SPECTRUM_CONSTANT_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_CONSTANT_SPECTRUM_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"

namespace specula {
  class ConstantSpectrum {
  public:
    SPECULA_CPU_GPU ConstantSpectrum(Float c) : c(c) {}

    SPECULA_CPU_GPU Float operator()(Float lambda) const { return c; }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return c; }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &) const;

  private:
    Float c;
  };
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_CONSTANT_SPECTRUM_HPP
