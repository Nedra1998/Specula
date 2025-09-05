#include "util/spectrum/spectrum.hpp"

#include "util/spectrum/blackbody_spectrum.hpp"
#include "util/spectrum/constant_spectrum.hpp"
#include "util/spectrum/densly_sampled_spectrum.hpp"
#include "util/spectrum/piecewise_linear_spectrum.hpp"
#include "util/spectrum/sampled_spectrum.hpp"

SPECULA_CPU_GPU specula::Float specula::Spectrum::operator()(Float lambda) const {
  auto op = [&](auto ptr) { return (*ptr)(lambda); };
  return dispatch(op);
}

SPECULA_CPU_GPU specula::Float specula::Spectrum::max_value() const {
  auto op = [&](auto ptr) { return ptr->max_value(); };
  return dispatch(op);
}

SPECULA_CPU_GPU specula::SampledSpectrum
specula::Spectrum::sample(const SampledWavelengths &lambda) const {
  auto op = [&](auto ptr) { return ptr->sample(lambda); };
  return dispatch(op);
}
