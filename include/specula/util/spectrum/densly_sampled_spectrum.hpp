#ifndef SPECULA_UTIL_SPECTRUM_DENSLY_SAMPLED_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_DENSLY_SAMPLED_SPECTRUM_HPP

#include <fmt/ranges.h>

#include "specula/util/hash.hpp"
#include "specula/util/spectrum/sampled_spectrum.hpp"
#include "specula/util/spectrum/sampled_wavelengths.hpp"
#include "specula/util/spectrum/spectrum.hpp"

namespace specula {
  class DenselySampledSpectrum {
  public:
    template <typename F>
    static DenselySampledSpectrum sample_function(F func, int lambda_min = LAMBDA_MIN,
                                                  int lambda_max = LAMBDA_MAX,
                                                  Allocator alloc = {}) {
      DenselySampledSpectrum s(lambda_min, lambda_max, alloc);
      for (int lambda = lambda_min; lambda <= lambda_max; ++lambda) {
        s.values[lambda - lambda_min] = func(lambda);
      }
      return s;
    }

    DenselySampledSpectrum(int lambda_min = LAMBDA_MAX, int lambda_max = LAMBDA_MAX,
                           Allocator alloc = {})
        : lambda_min(lambda_min), lambda_max(lambda_max),
          values(lambda_max - lambda_min + 1, alloc) {}
    DenselySampledSpectrum(Spectrum s, Allocator alloc)
        : DenselySampledSpectrum(s, LAMBDA_MIN, LAMBDA_MAX, alloc) {}

    DenselySampledSpectrum(Spectrum spec, int lambda_min = LAMBDA_MIN, int lambda_max = LAMBDA_MAX,
                           Allocator alloc = {})
        : lambda_min(lambda_min), lambda_max(lambda_max),
          values(lambda_max - lambda_min + 1, alloc) {
      ASSERT_GE(lambda_max, lambda_min);
      if (spec) {
        for (int lambda = lambda_min; lambda <= lambda_max; ++lambda) {
          values[lambda - lambda_min] = spec(static_cast<Float>(lambda));
        }
      }
    }

    DenselySampledSpectrum(const DenselySampledSpectrum &s, Allocator alloc)
        : lambda_min(s.lambda_min), lambda_max(s.lambda_max),
          values(s.values.begin(), s.values.end(), alloc) {}

    SPECULA_CPU_GPU bool operator==(const DenselySampledSpectrum &d) const {
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

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      DASSERT_GT(lambda, 0);
      int offset = std::lround(lambda) - lambda_min;
      if (offset < 0 || offset >= values.size()) {
        return 0;
      }
      return values[offset];
    }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const {
      return *std::ranges::max_element(values);
    }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        int offset = std::lround(lambda[i]) - lambda_min;
        if (offset < 0 || offset >= values.size()) {
          s[i] = 0;
        } else {
          s[i] = values[offset];
        }
      }
      return s;
    }

    SPECULA_CPU_GPU void scale(Float s) {
      for (Float &v : values) {
        v *= s;
      }
    }

  private:
    int lambda_min, lambda_max;
    pstd::vector<Float> values;

    friend struct std::hash<DenselySampledSpectrum>;
    friend struct fmt::formatter<DenselySampledSpectrum>;
  };
} // namespace specula

template <> struct std::hash<specula::DenselySampledSpectrum> {
  SPECULA_CPU_GPU size_t operator()(const specula::DenselySampledSpectrum &s) const {
    return specula::hash_buffer(s.values.data(), s.values.size());
  }
};

template <> struct fmt::formatter<specula::DenselySampledSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::DenselySampledSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ DenselySampledSpectrum lambdaMin={} lambdaMax={} values: {} ]",
                     v.lambda_min, v.lambda_max, fmt::join(v.values, ", "));
  }
};

#endif // SPECULA_UTIL_SPECTRUM_DENSLY_SAMPLED_SPECTRUM_HPP
