#include "specula/util/spectrum/sampled_spectrum.hpp"

#include "specula/util/colorspace.hpp"
#include "specula/util/spectrum/densly_sampled_spectrum.hpp"
#include "specula/util/spectrum/spectra.hpp"

SPECULA_CPU_GPU [[nodiscard]] specula::Xyz
specula::SampledSpectrum::to_xyz(const SampledWavelengths &lambda) const {
  SampledSpectrum x = spectra::X().sample(lambda);
  SampledSpectrum y = spectra::Y().sample(lambda);
  SampledSpectrum z = spectra::Z().sample(lambda);

  SampledSpectrum pdf = lambda.pdf();

  return Xyz(safe_div(x * *this, pdf).average(), safe_div(y * *this, pdf).average(),
             safe_div(z * *this, pdf).average()) /
         CIE_Y_INTEGRAL;
}

SPECULA_CPU_GPU [[nodiscard]] specula::Rgb
specula::SampledSpectrum::to_rgb(const SampledWavelengths &lambda, const RgbColorSpace &cs) const {
  Xyz xyz = to_xyz(lambda);
  return cs.to_rgb(xyz);
}

SPECULA_CPU_GPU [[nodiscard]] specula::Float
specula::SampledSpectrum::y(const SampledWavelengths &lambda) const {
  SampledSpectrum ys = spectra::Y().sample(lambda);
  SampledSpectrum pdf = lambda.pdf();

  return safe_div(ys * *this, pdf).average() / CIE_Y_INTEGRAL;
}
