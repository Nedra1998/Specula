#ifndef INCLUDE_SPECTRUM_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_SPECTRUM_HPP_

#include "specula/specula.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/tagged_pointer.hpp"

namespace specula {
  class BlackbodySpectrum;
  class ConstantSpectrum;
  class PiecewiseLinearSpectrum;
  class DenslySampledSpectrum;
  class RgbAlbedoSpectrum;
  class RgbUnboundedSpectrum;
  class RgbIlluminantSpectrum;

  class Spectrum
      : public TaggedPointer<ConstantSpectrum, DenslySampledSpectrum, PiecewiseLinearSpectrum,
                             // RgbAlbedoSpectrum, RgbUnboundedSpectrum, RgbIlluminantSpectrum,
                             BlackbodySpectrum> {
  public:
    using TaggedPointer::TaggedPointer;

    SPECULA_CPU_GPU Float operator()(Float lambda) const;
    SPECULA_CPU_GPU Float max_value() const;
    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const;
  };
} // namespace specula

#endif // INCLUDE_SPECTRUM_SPECTRUM_HPP_
