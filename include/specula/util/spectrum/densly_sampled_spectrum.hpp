#ifndef INCLUDE_SPECTRUM_DENSLY_SAMPLED_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_DENSLY_SAMPLED_SPECTRUM_HPP_

#include <fmt/ranges.h>

#include "specula.hpp"
#include "util/hash.hpp"
#include "util/pstd/vector.hpp"
#include "util/spectrum/constants.hpp"
#include "util/spectrum/sampled_spectrum.hpp"
#include "util/spectrum/sampled_wavelengths.hpp"
#include "util/spectrum/spectrum.hpp"

namespace specula {

  class DenslySampledSpectrum {
  public:
    DenslySampledSpectrum(int lambda_min = LAMBDA_MIN, int lambda_max = LAMBDA_MAX,
                          Allocator alloc = {})
        : lambda_min(lambda_min), lambda_max(lambda_max),
          values(lambda_max - lambda_min + 1, alloc) {}

    DenslySampledSpectrum(Spectrum s, Allocator alloc)
        : specula::DenslySampledSpectrum(s, LAMBDA_MIN, LAMBDA_MAX, alloc) {}

    DenslySampledSpectrum(const DenslySampledSpectrum &s, Allocator alloc)
        : lambda_min(s.lambda_min), lambda_max(s.lambda_max),
          values(s.values.begin(), s.values.end(), alloc) {}

    DenslySampledSpectrum(Spectrum spec, int lambda_min = LAMBDA_MIN, int lambda_max = LAMBDA_MAX,
                          Allocator alloc = {})
        : lambda_min(lambda_min), lambda_max(lambda_max),
          values(lambda_max - lambda_min + 1, alloc) {
      ASSERT_GE(lambda_max, lambda_min);
      if (spec) {
        for (int lambda = lambda_min; lambda <= lambda_max; ++lambda) {
          values[lambda - lambda_min] = spec(lambda);
        }
      }
    }

    template <typename F>
    static DenslySampledSpectrum sample_function(F func, int lambda_min = LAMBDA_MIN,
                                                 int lambda_max = LAMBDA_MAX,
                                                 Allocator alloc = {}) {
      DenslySampledSpectrum s(lambda_min, lambda_max, alloc);
      for (int lambda = lambda_min; lambda <= lambda_max; ++lambda) {
        s.values[lambda - lambda_min] = func(lambda);
      }
      return s;
    }

    SPECULA_CPU_GPU void scale(Float s) {
      for (Float &v : values) {
        v *= s;
      }
    }

    SPECULA_CPU_GPU SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;

      for (int i = 0; i < NSpectrumSamples; ++i) {
        int offset = std::lround(lambda[i]) - lambda_min;
        if (offset < 0 || offset >= values.size()) {
          s[i] = 0;
        } else {
          s[i] = values[offset];
        }
      }
      return s;
    }

    SPECULA_CPU_GPU Float max_value() const {
      return *std::max_element(values.begin(), values.end());
    }

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      DASSERT_GE(lambda, 0);
      int offset = std::lround(lambda) - lambda_min;
      if (offset < 0 || offset >= values.size()) {
        return 0;
      }
      return values[offset];
    }

    SPECULA_CPU_GPU bool operator==(const DenslySampledSpectrum &d) const {
      if (lambda_min != d.lambda_min || lambda_max != d.lambda_max ||
          values.size() != d.values.size()) {
        return false;
      }
      for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] != d.values[i]) {
          return false;
        }
      }

      return true;
    }

  private:
    int lambda_min, lambda_max;
    pstd::vector<Float> values;

    friend struct std::hash<specula::DenslySampledSpectrum>;
    friend struct fmt::formatter<DenslySampledSpectrum>;
  };
} // namespace specula

template <> struct std::hash<specula::DenslySampledSpectrum> {
  SPECULA_CPU_GPU size_t operator()(const specula::DenslySampledSpectrum &s) const {
    return specula::hash_buffer(s.values.data(), s.values.size());
  }
};

template <> struct fmt::formatter<specula::DenslySampledSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::DenslySampledSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(),
                     "[ DenslySampledSpectrum lambda_min={:d} lambda_max={:d} values={} ]",
                     v.lambda_min, v.lambda_max, v.values);
  }
};

#endif // INCLUDE_SPECTRUM_DENSLY_SAMPLED_SPECTRUM_HPP_
