#ifndef INCLUDE_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP_

#include "specula.hpp"
#include "specula/util/pstd/vector.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "util/spectrum/spectra.hpp"

namespace specula {
  class Spectrum;

  class PiecewiseLinearSpectrum {
  public:
    PiecewiseLinearSpectrum() = default;
    PiecewiseLinearSpectrum(pstd::span<const Float> lambdas, pstd::span<const Float> values,
                            Allocator alloc = {});

    static pstd::optional<Spectrum> read(const std::string &filename, Allocator alloc);
    static PiecewiseLinearSpectrum *from_interleaved(pstd::span<const Float> samples,
                                                     bool normalize, Allocator alloc);

    SPECULA_CPU_GPU void scale(Float s) {
      for (Float &v : values) {
        v *= s;
      }
    }

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        s[i] = (*this)(lambda[i]);
      }
      return s;
    }

    SPECULA_CPU_GPU Float max_value() const;

    SPECULA_CPU_GPU Float operator()(Float lambda) const;

  private:
    pstd::vector<Float> lambdas, values;

    friend struct fmt::formatter<PiecewiseLinearSpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::PiecewiseLinearSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::PiecewiseLinearSpectrum &v, FormatContext &ctx) const {
    std::string name = specula::find_matching_named_spectrum(&v);
    if (!name.empty()) {
      return format_to(ctx.out(), "{}", name);
    }
    format_to(ctx.out(), "[ PiecewiseLinearSpectrum ");
    for (size_t i = 0; i < v.lambdas.size(); ++i) {
      format_to(ctx.out(), "{}={} ", v.lambdas[i], v.values[i]);
    }
    return format_to(ctx.out(), "]");
  }
};

#endif // INCLUDE_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP_
