#ifndef INCLUDE_SPECTRUM_SAMPLED_SPECTRUM_HPP_
#define INCLUDE_SPECTRUM_SAMPLED_SPECTRUM_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/float.hpp"
#include "specula/util/pstd/array.hpp"
#include "specula/util/pstd/span.hpp"
#include "specula/util/spectrum/constants.hpp"
#include "util/color/rgb.hpp"
#include "util/color/xyz.hpp"

namespace specula {
  class RgbColorSpace;
  class SampledWavelengths;

  class SampledSpectrum {
  public:
    SampledSpectrum() = default;
    SPECULA_CPU_GPU explicit SampledSpectrum(Float c) { values.fill(c); }
    SPECULA_CPU_GPU SampledSpectrum(pstd::span<const Float> v) {
      DASSERT_EQ(NSpectrumSamples, v.size());
      for (int i = 0; i < NSpectrumSamples; ++i) {
        values[i] = v[i];
      }
    }

    SPECULA_CPU_GPU bool has_nans() const {
      for (int i = 0; i < NSpectrumSamples; ++i) {
        if (isnan(values[i])) {
          return true;
        }
      }
      return false;
    }

    SPECULA_CPU_GPU Xyz to_xyz(const SampledWavelengths &lambda) const;
    SPECULA_CPU_GPU Rgb to_rgb(const SampledWavelengths &lambda, const RgbColorSpace &cs) const;
    SPECULA_CPU_GPU Float y(const SampledWavelengths &lambda) const;

    SPECULA_CPU_GPU Float min_component_value() const {
      Float m = values[0];
      for (int i = 1; i < NSpectrumSamples; ++i) {
        m = std::min(m, values[i]);
      }
      return m;
    }

    SPECULA_CPU_GPU Float max_component_value() const {
      Float m = values[0];
      for (int i = 1; i < NSpectrumSamples; ++i) {
        m = std::max(m, values[i]);
      }
      return m;
    }

    SPECULA_CPU_GPU Float average() const {
      Float sum = values[0];
      for (int i = 1; i < NSpectrumSamples; ++i) {
        sum += values[i];
      }
      return sum / NSpectrumSamples;
    }

    SPECULA_CPU_GPU explicit operator bool() const {
      for (int i = 0; i < NSpectrumSamples; ++i) {
        if (values[i] != 0) {
          return true;
        }
      }
      return false;
    }

    SPECULA_CPU_GPU Float operator[](int i) const {
      DASSERT(i >= 0 && i < NSpectrumSamples);
      return values[i];
    }

    SPECULA_CPU_GPU Float &operator[](int i) {
      DASSERT(i >= 0 && i < NSpectrumSamples);
      return values[i];
    }

    SPECULA_CPU_GPU bool operator==(const SampledSpectrum &s) const { return values == s.values; }
    SPECULA_CPU_GPU bool operator!=(const SampledSpectrum &s) const { return values != s.values; }

    SPECULA_CPU_GPU SampledSpectrum operator-() const {
      SampledSpectrum ret;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        ret.values[i] = -values[i];
      }
      return ret;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator+=(const SampledSpectrum &s) {
      for (int i = 0; i < NSpectrumSamples; ++i) {
        values[i] += s.values[i];
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator+(const SampledSpectrum &s) const {
      SampledSpectrum ret = *this;
      return ret += s;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator-=(const SampledSpectrum &s) {
      for (int i = 0; i < NSpectrumSamples; ++i) {
        values[i] -= s.values[i];
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator-(const SampledSpectrum &s) const {
      SampledSpectrum ret = *this;
      return ret -= s;
    }

    SPECULA_CPU_GPU friend SampledSpectrum operator-(Float a, const SampledSpectrum &s) {
      DASSERT(!isnan(a));
      SampledSpectrum ret;
      for (int i = 0; i < NSpectrumSamples; ++i) {
        ret.values[i] = a - s.values[i];
      }
      return ret;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator*=(const SampledSpectrum &s) {
      for (int i = 0; i < NSpectrumSamples; ++i) {
        values[i] *= s.values[i];
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator*(const SampledSpectrum &s) const {
      SampledSpectrum ret = *this;
      return ret *= s;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator*=(Float a) {
      DASSERT(!isnan(a));
      for (int i = 0; i < NSpectrumSamples; ++i) {
        values[i] *= a;
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator*(Float a) const {
      SampledSpectrum ret = *this;
      return ret *= a;
    }

    SPECULA_CPU_GPU friend SampledSpectrum operator*(Float a, const SampledSpectrum &s) {
      return s * a;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator/=(const SampledSpectrum &s) {
      for (int i = 0; i < NSpectrumSamples; ++i) {
        DASSERT_NE(0, s.values[i]);
        values[i] /= s.values[i];
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator/(const SampledSpectrum &s) const {
      SampledSpectrum ret = *this;
      return ret /= s;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator/=(Float a) {
      DASSERT(!isnan(a));
      for (int i = 0; i < NSpectrumSamples; ++i) {
        values[i] /= a;
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator/(Float a) const {
      SampledSpectrum ret = *this;
      return ret /= a;
    }

  private:
    // friend struct Soa<SampledSpectrum>;
    pstd::array<Float, NSpectrumSamples> values;

    friend struct fmt::formatter<SampledSpectrum>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::SampledSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::SampledSpectrum &v, FormatContext &ctx) const {
    // TODO: Implement proper formatting
    return format_to(ctx.out(), "[ {} ]", v.values);
  }
};

#endif // INCLUDE_SPECTRUM_SAMPLED_SPECTRUM_HPP_
