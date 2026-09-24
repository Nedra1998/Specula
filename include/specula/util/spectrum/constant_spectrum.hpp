#ifndef SPECULA_UTIL_SPECTRUM_CONSTANT_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_CONSTANT_SPECTRUM_HPP

#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/spectrum.hpp"

namespace specula {
  class ConstantSpectrum {
  public:
    SPECULA_CPU_GPU ConstantSpectrum(Float c) : c(c) {}

    SPECULA_CPU_GPU Float operator()(Float lambda) const { return c; }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return c; }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum
    sample(const SampledWavelengths & /*unused*/) const {
      return SampledSpectrum(c);
    }

  private:
    Float c;

    friend struct fmt::formatter<ConstantSpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::ConstantSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::ConstantSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ ConstantSpectrum c={} ]", v.c);
  }
};

#endif // SPECULA_UTIL_SPECTRUM_CONSTANT_SPECTRUM_HPP
