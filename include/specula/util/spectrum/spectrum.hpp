#ifndef SPECULA_UTIL_SPECTRUM_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_SPECTRUM_HPP

#include "specula/macros.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/taggedptr.hpp"

namespace specula {
  class BlackbodySpectrum;
  class ConstantSpectrum;
  class PiecewiseLinearSpectrum;
  class DenselySampledSpectrum;
  class RgbAlbedoSpectrum;
  class RgbUnboundedSpectrum;
  class RgbIlluminantSpectrum;

  class SampledSpectrum;
  class SampledWavelengths;

  class Spectrum
      : public TaggedPointer<ConstantSpectrum, DenselySampledSpectrum, PiecewiseLinearSpectrum,
                             RgbAlbedoSpectrum, RgbUnboundedSpectrum, RgbIlluminantSpectrum,
                             BlackbodySpectrum> {
  public:
    using TaggedPointer::TaggedPointer;

    SPECULA_CPU_GPU Float operator()(Float lambda) const;
    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const;
    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const;
  };

  SPECULA_CPU_GPU inline Float inner_product(Spectrum f, Spectrum g) {
    Float integral = 0;
    for (Float lambda = LAMBDA_MIN; lambda <= LAMBDA_MAX; ++lambda) {
      integral += f(lambda) * g(lambda);
    }
    return integral;
  }
} // namespace specula

#include "specula/util/spectrum/blackbody_spectrum.hpp"
#include "specula/util/spectrum/constant_spectrum.hpp"
#include "specula/util/spectrum/densely_sampled_spectrum.hpp"
#include "specula/util/spectrum/piecewise_linear_spectrum.hpp"
#include "specula/util/spectrum/rgb_albedo_spectrum.hpp"
#include "specula/util/spectrum/rgb_illuminant_spectrum.hpp"
#include "specula/util/spectrum/rgb_unbounded_spectrum.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"

SPECULA_CPU_GPU inline specula::Float specula::Spectrum::operator()(Float lambda) const {
  auto op = [&](auto ptr) { return (*ptr)(lambda); };
  return dispatch(op);
}

SPECULA_CPU_GPU inline specula::Float specula::Spectrum::max_value() const {
  auto op = [&](auto ptr) { return ptr->max_value(); };
  return dispatch(op);
}

SPECULA_CPU_GPU inline specula::SampledSpectrum
specula::Spectrum::sample(const SampledWavelengths &lambda) const {
  auto op = [&](auto ptr) { return ptr->sample(lambda); };
  return dispatch(op);
}

#endif // SPECULA_UTIL_SPECTRUM_SPECTRUM_HPP
