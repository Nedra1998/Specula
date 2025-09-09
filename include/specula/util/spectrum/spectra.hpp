#ifndef INCLUDE_SPECTRUM_SPECTRA_HPP_
#define INCLUDE_SPECTRUM_SPECTRA_HPP_

#include "specula.hpp"
#include "util/spectrum/densly_sampled_spectrum.hpp"

namespace specula {
  class Spectrum;

  namespace spectra {
    void init(Allocator alloc);

    DenslySampledSpectrum D(Float temperature, Allocator alloc);

    SPECULA_CPU_GPU inline const DenslySampledSpectrum &X() {
#ifdef SPECULA_IS_GPU_CODE
      extern SPECULA_GPU DenslySampledSpectrum *xGPU;
      return *xGPU;
#else
      extern DenslySampledSpectrum *x;
      return *x;
#endif
    }

    SPECULA_CPU_GPU inline const DenslySampledSpectrum &Y() {
#ifdef SPECULA_IS_GPU_CODE
      extern SPECULA_GPU DenslySampledSpectrum *yGPU;
      return *yGPU;
#else
      extern DenslySampledSpectrum *y;
      return *y;
#endif
    }

    SPECULA_CPU_GPU inline const DenslySampledSpectrum &Z() {
#ifdef SPECULA_IS_GPU_CODE
      extern SPECULA_GPU DenslySampledSpectrum *zGPU;
      return *zGPU;
#else
      extern DenslySampledSpectrum *z;
      return *z;
#endif
    }
  } // namespace spectra

  // TODO: If possible change the spectrum names into an enum
  Spectrum get_named_spectrum(std::string name);
  std::string find_matching_named_spectrum(Spectrum s);
} // namespace specula

#endif // INCLUDE_SPECTRUM_SPECTRA_HPP_
