#include "util/spectrum/sampled_spectrum.hpp"

#include "util/color/rgb.hpp"
#include "util/color/xyz.hpp"
#include "util/colorspace.hpp"
#include "util/spectrum/constants.hpp"
#include "util/spectrum/sampled_wavelengths.hpp"
#include "util/spectrum/spectra.hpp"

SPECULA_CPU_GPU specula::Xyz
specula::SampledSpectrum::to_xyz(const SampledWavelengths &lambda) const {
  SampledSpectrum X = spectra::X().sample(lambda);
  SampledSpectrum Y = spectra::Y().sample(lambda);
  SampledSpectrum Z = spectra::Z().sample(lambda);

  SampledSpectrum pdf = lambda.pdf();
  return Xyz(safe_div(X * *this, pdf).average(), safe_div(Y * *this, pdf).average(),
             safe_div(Z * *this, pdf).average()) /
         CIE_Y_INTEGRAL;
}

SPECULA_CPU_GPU specula::Rgb specula::SampledSpectrum::to_rgb(const SampledWavelengths &lambda,
                                                              const RgbColorSpace &cs) const {
  Xyz xyz = to_xyz(lambda);
  return cs.to_rgb(xyz);
}

SPECULA_CPU_GPU specula::Float specula::SampledSpectrum::y(const SampledWavelengths &lambda) const {
  SampledSpectrum Y = spectra::Y().sample(lambda);
  SampledSpectrum pdf = lambda.pdf();
  return safe_div(Y * *this, pdf).average() / CIE_Y_INTEGRAL;
}
