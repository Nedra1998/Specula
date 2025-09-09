#ifndef INCLUDE_SPECTRUM_FUNCTIONS_HPP_
#define INCLUDE_SPECTRUM_FUNCTIONS_HPP_

#include "specula.hpp"
#include "util/color/xyz.hpp"
#include "util/float.hpp"
#include "util/math/functions.hpp"

namespace specula {
  class Spectrum;

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

  Float spectrum_to_photometric(Spectrum s);
  Xyz spectrum_to_xyz(Spectrum s);
} // namespace specula

#endif // INCLUDE_SPECTRUM_FUNCTIONS_HPP_
