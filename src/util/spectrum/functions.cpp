#include "util/spectrum/functions.hpp"

#include "util/spectrum/rgb_illuminant_spectrum.hpp"
#include "util/spectrum/spectra.hpp"
#include "util/spectrum/spectrum.hpp"

specula::Float specula::spectrum_to_photometric(Spectrum s) {
  if (s.is<RgbIlluminantSpectrum>()) {
    s = s.cast<RgbIlluminantSpectrum>()->illuminant();
  }
  return inner_product(&spectra::Y(), s);
}
