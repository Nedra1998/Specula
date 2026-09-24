#ifndef SPECULA_UTIL_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP

#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/spectrum/spectra.hpp"
#include "specula/util/spectrum/spectrum.hpp"

namespace specula {
  class PiecewiseLinearSpectrum {
  public:
    static pstd::optional<Spectrum> read(const std::string &filename, Allocator alloc);
    static PiecewiseLinearSpectrum *from_interleaved(pstd::span<const Float> samples,
                                                     bool normalize, Allocator alloc);

    PiecewiseLinearSpectrum() = default;
    PiecewiseLinearSpectrum(pstd::span<const Float> lambda, pstd::span<const Float> values,
                            Allocator alloc = {});

    SPECULA_CPU_GPU void scale(Float s) {
      for (Float &v : values) {
        v *= s;
      }
    }

    SPECULA_CPU_GPU Float operator()(Float lambda) const;

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const;

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;

      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = (*this)(lambda[i]);
      }

      return s;
    }

  private:
    pstd::vector<Float> lambdas, values;

    friend struct fmt::formatter<PiecewiseLinearSpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::PiecewiseLinearSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::PiecewiseLinearSpectrum &v, FormatContext &ctx) const {
    std::string name = specula::find_matching_named_spectrum(this);
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

#endif // SPECULA_UTIL_SPECTRUM_PIECEWISE_LINEAR_SPECTRUM_HPP
