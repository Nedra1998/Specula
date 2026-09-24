#ifndef SPECULA_UTIL_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_HPP

// IWYU pragma: begin_exports
#include "specula/util/spectrum/blackbody_spectrum.hpp"
#include "specula/util/spectrum/constant_spectrum.hpp"
#include "specula/util/spectrum/densly_sampled_spectrum.hpp"
#include "specula/util/spectrum/functions.hpp"
#include "specula/util/spectrum/piecewise_linear_spectrum.hpp"
#include "specula/util/spectrum/rgb_albedo_spectrum.hpp"
#include "specula/util/spectrum/rgb_illuminant_spectrum.hpp"
#include "specula/util/spectrum/rgb_unbounded_spectrum.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/spectrum/spectra.hpp"
#include "specula/util/spectrum/spectrum.hpp"
// IWYU pragma: end_exports

namespace specula {
  SPECULA_CPU_GPU inline Float Spectrum::operator()(Float lambda) const {
    auto op = [&](auto ptr) { return (*ptr)(lambda); };
    return dispatch(op);
  }

  SPECULA_CPU_GPU inline Float Spectrum::max_value() const {
    auto op = [&](auto ptr) { return ptr->max_value(); };
    return dispatch(op);
  }

  SPECULA_CPU_GPU inline SampledSpectrum Spectrum::sample(const SampledWavelengths &lambda) const {
    auto op = [&](auto ptr) { return ptr->sample(lambda); };
    return dispatch(op);
  }
} // namespace specula

template <> struct fmt::formatter<specula::Spectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Spectrum &v, FormatContext &ctx) const {
    if (!v.ptr()) {
      return format_to(ctx.out(), "(nullptr)");
    }
    auto ts = [&](auto ptr) { return format_to(ctx.out(), "{}", *ptr); };
    return v.dispatch_cpu(ts);
  }
};

#endif // SPECULA_UTIL_SPECTRUM_HPP
