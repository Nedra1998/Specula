#ifndef SPECULA_UTIL_SPECTRUM_FUNCTIONS_HPP
#define SPECULA_UTIL_SPECTRUM_FUNCTIONS_HPP

#include <cmath>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/color.hpp"
#include "specula/util/spectrum/spectrum.hpp"

namespace specula {
  // TODO: Move to sampling.hpp when it is implemented
  SPECULA_CPU_GPU inline Float sample_visible_wavelengths(Float u) {
    return 538 - 138.888889f * std::atanh(0.85691062f - 1.82750197f * u);
  }

  // TODO: Move to sampling.hpp when it is implemented
  SPECULA_CPU_GPU inline Float visible_wavelengths_pdf(Float lambda) {
    if (lambda < 360 || lambda > 830) {
      return 0;
    }
    return 0.0039398042f / sqr(std::cosh(0.0072f * (lambda - 538)));
  }

  Float spectrum_to_photometric(Spectrum s);
  Xyz spectrum_to_xyz(Spectrum s);

  SPECULA_CPU_GPU inline Float blackbody(Float lambda, Float t) {
    if (t <= 0) {
      return 0;
    }

    const Float c = 299792458.f;
    const Float h = 6.62606957e-34f;
    const Float kb = 1.3806488e-23f;

    Float l = lambda * 1e-9f;
    Float le = (2 * h * c * c) / (pow<5>(l) * (fast_exp((h * c) / (l * kb * t)) - 1));
    ASSERT(!isnan(le));
    return le;
  }
} // namespace specula

#endif // SPECULA_UTIL_SPECTRUM_FUNCTIONS_HPP
