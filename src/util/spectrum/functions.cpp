#include "util/spectrum/functions.hpp"

#include "util/color/xyz.hpp"
#include "util/spectrum/constants.hpp"
#include "util/spectrum/rgb_illuminant_spectrum.hpp"
#include "util/spectrum/spectra.hpp"
#include "util/spectrum/spectrum.hpp"

specula::Float specula::spectrum_to_photometric(Spectrum s) {
  if (s.is<RgbIlluminantSpectrum>()) {
    s = s.cast<RgbIlluminantSpectrum>()->illuminant();
  }
  return inner_product(&spectra::Y(), s);
}

specula::Xyz specula::spectrum_to_xyz(Spectrum s) {
  return Xyz(inner_product(&spectra::X(), s), inner_product(&spectra::Y(), s),
             inner_product(&spectra::Z(), s)) /
         CIE_Y_INTEGRAL;
}
