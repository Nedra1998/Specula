#ifndef SPECULA_UTIL_SPECTRUM_SPECTRA_HPP
#define SPECULA_UTIL_SPECTRUM_SPECTRA_HPP

#include "specula/types.hpp"
#include "specula/util/spectrum/spectrum.hpp"

namespace specula {
  namespace spectra {
    void init(Allocator alloc);

    DenselySampledSpectrum D(Float temperature, Allocator alloc);

    SPECULA_CPU_GPU inline const DenselySampledSpectrum &X() {
#ifdef SPEUCLA_IS_GPU_CODE
      extern SPECULA_GPU DenselySampledSpectrum *xGPU;
      return *xGPU;
#else
      extern DenselySampledSpectrum *x;
      return *x;
#endif
    }

    SPECULA_CPU_GPU inline const DenselySampledSpectrum &Y() {
#ifdef SPEUCLA_IS_GPU_CODE
      extern SPECULA_GPU DenselySampledSpectrum *yGPU;
      return *yGPU;
#else
      extern DenselySampledSpectrum *y;
      return *y;
#endif
    }

    SPECULA_CPU_GPU inline const DenselySampledSpectrum &Z() {
#ifdef SPEUCLA_IS_GPU_CODE
      extern SPECULA_GPU DenselySampledSpectrum *zGPU;
      return *zGPU;
#else
      extern DenselySampledSpectrum *z;
      return *z;
#endif
    }
  } // namespace spectra

  Spectrum get_named_spectrum(const std::string &name);
  std::string find_matching_named_spectrum(Spectrum s);
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_SPECTRA_HPP
