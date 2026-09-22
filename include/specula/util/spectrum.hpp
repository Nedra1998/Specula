#ifndef SPECULA_UTIL_SPECTRUM_HPP
#define SPECULA_UTIL_SPECTRUM_HPP

#include <fmt/ranges.h>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/color.hpp"
#include "specula/util/hash.hpp"
#include "specula/util/taggedptr.hpp"

namespace specula {
  static constexpr Float LAMBDA_MIN = 360, LAMBDA_MAX = 830;
  static constexpr int N_SPECTRUM_SAMPLES = 4;
  static constexpr Float CIE_Y_INTEGRAL = 106.856895;

  class RgbColorSpace;
  class BlackbodySpectrum;
  class ConstantSpectrum;
  class PiecewiseLinearSpectrum;
  class DenselySampledSpectrum;
  class RgbAlbedoSpectrum;
  class RgbUnboundedSpectrum;
  class RgbIlluminantSpectrum;

  class SampledSpectrum;
  class SampledWavelengths;

  // TODO: Move these into the sampling.hpp header when that is implemented
  SPECULA_CPU_GPU inline Float sample_visible_wavelengths(Float u);
  SPECULA_CPU_GPU inline Float visible_wavelengths_pdf(Float lambda);

  SPECULA_CPU_GPU inline Float blackbody(Float lambda, Float t);

  namespace spectra {
    void init(Allocator alloc);

    DenselySampledSpectrum D(Float temperature, Allocator alloc);

    SPECULA_CPU_GPU inline const DenselySampledSpectrum &X() {
#ifdef SPEUCLA_IS_GPU_CODE
      extern SPECULA_GPU DenselySampledSpectrum *xGPU;
      return *xGPU;
#else
      extern DenselySampledSpectrum *x;
      return *x;
#endif
    }

    SPECULA_CPU_GPU inline const DenselySampledSpectrum &Y() {
#ifdef SPEUCLA_IS_GPU_CODE
      extern SPECULA_GPU DenselySampledSpectrum *yGPU;
      return *yGPU;
#else
      extern DenselySampledSpectrum *y;
      return *y;
#endif
    }

    SPECULA_CPU_GPU inline const DenselySampledSpectrum &Z() {
#ifdef SPEUCLA_IS_GPU_CODE
      extern SPECULA_GPU DenselySampledSpectrum *zGPU;
      return *zGPU;
#else
      extern DenselySampledSpectrum *z;
      return *z;
#endif
    }
  } // namespace spectra

  class Spectrum
      : public TaggedPointer<ConstantSpectrum, DenselySampledSpectrum, PiecewiseLinearSpectrum,
                             RgbAlbedoSpectrum, RgbUnboundedSpectrum, RgbIlluminantSpectrum,
                             BlackbodySpectrum> {
  public:
    using TaggedPointer::TaggedPointer;

    SPECULA_CPU_GPU Float operator()(Float lambda) const;
    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const;
    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const;
  };

  class SampledSpectrum {
  public:
    SampledSpectrum() = default;
    SPECULA_CPU_GPU explicit SampledSpectrum(Float c) { values.fill(c); }
    SPECULA_CPU_GPU SampledSpectrum(pstd::span<const Float> v) {
      DASSERT_EQ(N_SPECTRUM_SAMPLES, v.size());
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] = v[i];
      }
    }

    SPECULA_CPU_GPU explicit operator bool() const {
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        if (values[i] != 0) {
          return true;
        }
      }
      return false;
    }

    SPECULA_CPU_GPU Float operator[](int i) const {
      DASSERT(i >= 0 && i < N_SPECTRUM_SAMPLES);
      return values[i];
    }
    SPECULA_CPU_GPU Float &operator[](int i) {
      DASSERT(i >= 0 && i < N_SPECTRUM_SAMPLES);
      return values[i];
    }

    SPECULA_CPU_GPU bool operator==(const SampledSpectrum &s) const { return values == s.values; }
    SPECULA_CPU_GPU bool operator!=(const SampledSpectrum &s) const { return values != s.values; }

    SPECULA_CPU_GPU SampledSpectrum operator-() const {
      SampledSpectrum ret;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        ret.values[i] = -values[i];
      }
      return ret;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator+=(const SampledSpectrum &s) {
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] += s.values[i];
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator+(const SampledSpectrum &s) const {
      SampledSpectrum ret = *this;
      return ret += s;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator-=(const SampledSpectrum &s) {
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
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
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        ret.values[i] = a - s.values[i];
      }
      return ret;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator*=(const SampledSpectrum &s) {
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
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
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] *= a;
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator*(Float a) const {
      SampledSpectrum ret = *this;
      return ret *= a;
    }

    SPECULA_CPU_GPU SampledSpectrum &operator/=(const SampledSpectrum &s) {
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
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
      DASSERT_NE(0, a);
      DASSERT(!isnan(a));
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        values[i] /= a;
      }
      return *this;
    }

    SPECULA_CPU_GPU SampledSpectrum operator/(Float a) const {
      SampledSpectrum ret = *this;
      return ret /= a;
    }

    SPECULA_CPU_GPU [[nodiscard]] bool has_nans() const {
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        if (isnan(values[i])) {
          return true;
        }
      }
      return false;
    }

    SPECULA_CPU_GPU [[nodiscard]] Xyz to_xyz(const SampledWavelengths &lambda) const;
    SPECULA_CPU_GPU [[nodiscard]] Rgb to_rgb(const SampledWavelengths &lambda,
                                             const RgbColorSpace &cs) const;
    SPECULA_CPU_GPU [[nodiscard]] Float y(const SampledWavelengths &lambda) const;

    SPECULA_CPU_GPU [[nodiscard]] Float min_component_value() const {
      Float m = values[0];
      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        m = std::min(m, values[i]);
      }
      return m;
    }

    SPECULA_CPU_GPU [[nodiscard]] Float max_component_value() const {
      Float m = values[0];
      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        m = std::max(m, values[i]);
      }
      return m;
    }

    SPECULA_CPU_GPU [[nodiscard]] Float average() const {
      Float sum = values[0];
      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        sum += values[i];
      }
      return sum / N_SPECTRUM_SAMPLES;
    }

  private:
    pstd::array<Float, N_SPECTRUM_SAMPLES> values;

    friend struct fmt::formatter<SampledSpectrum>;
  };

  SPECULA_CPU_GPU inline SampledSpectrum operator*(Float a, const SampledSpectrum &s) {
    return s * a;
  }

  SPECULA_CPU_GPU inline SampledSpectrum safe_div(SampledSpectrum a, SampledSpectrum b) {
    SampledSpectrum r;
    for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
      r[i] = (b[i] != 0) ? a[i] / b[i] : 0.;
    }
    return r;
  }

  SPECULA_CPU_GPU inline SampledSpectrum clamp_zero(const SampledSpectrum &s) {
    SampledSpectrum ret;
    for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
      ret[i] = std::max<Float>(0, s[i]);
    }
    DASSERT(!ret.has_nans());
    return ret;
  }

  SPECULA_CPU_GPU inline SampledSpectrum safe_sqrt(const SampledSpectrum &s) {
    SampledSpectrum ret;
    for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
      ret[i] = safe_sqrt(s[i]);
    }
    DASSERT(!ret.has_nans());
    return ret;
  }

  SPECULA_CPU_GPU inline SampledSpectrum pow(const SampledSpectrum &s, Float e) {
    SampledSpectrum ret;
    for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
      ret[i] = std::pow(s[i], e);
    }
    DASSERT(!ret.has_nans());
    return ret;
  }

  SPECULA_CPU_GPU inline SampledSpectrum exp(const SampledSpectrum &s) {
    SampledSpectrum ret;
    for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
      ret[i] = std::exp(s[i]);
    }
    DASSERT(!ret.has_nans());
    return ret;
  }

  SPECULA_CPU_GPU inline SampledSpectrum fast_exp(const SampledSpectrum &s) {
    SampledSpectrum ret;
    for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
      ret[i] = fast_exp(s[i]);
    }
    DASSERT(!ret.has_nans());
    return ret;
  }

  SPECULA_CPU_GPU inline SampledSpectrum bilerp(pstd::array<Float, 2> p,
                                                pstd::span<const SampledSpectrum> v) {
    return (1 - p[0]) * (1 - p[1]) * v[0] + p[0] * (1 - p[1]) * v[1] + (1 - p[0]) * p[1] * v[2] +
           p[0] * p[1] * v[3];
  }

  SPECULA_CPU_GPU inline SampledSpectrum lerp(Float t, const SampledSpectrum &s1,
                                              const SampledSpectrum &s2) {
    return (1 - t) * s1 + t * s2;
  }

  class SampledWavelengths {
  public:
    SPECULA_CPU_GPU static SampledWavelengths sample_uniform(Float u, Float lambda_min = LAMBDA_MAX,
                                                             Float lambda_max = LAMBDA_MAX) {
      SampledWavelengths swl;

      swl.lambda[0] = lerp(u, lambda_min, lambda_max);

      Float delta = (lambda_max - lambda_min) / N_SPECTRUM_SAMPLES;
      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        swl.lambda[i] = swl.lambda[i - 1] + delta;
        if (swl.lambda[i] > lambda_max) {
          swl.lambda[i] = lambda_min + (swl.lambda[i] - lambda_max);
        }
      }

      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        swl.pdf_[i] = 1 / (lambda_max - lambda_min);
      }

      return swl;
    }

    SPECULA_CPU_GPU static SampledWavelengths sample_visible(Float u) {
      SampledWavelengths swl;

      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        Float up = u + Float(i) / N_SPECTRUM_SAMPLES;
        if (up > 1) {
          up -= 1;
        }

        swl.lambda[i] = sample_visible_wavelengths(up);
        swl.pdf_[i] = visible_wavelengths_pdf(swl.lambda[i]);
      }

      return swl;
    }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum pdf() const { return {pdf_}; }

    SPECULA_CPU_GPU Float operator[](int i) const { return lambda[i]; }
    SPECULA_CPU_GPU Float &operator[](int i) { return lambda[i]; }

    SPECULA_CPU_GPU bool operator==(const SampledWavelengths &swl) const {
      return lambda == swl.lambda && pdf_ == swl.pdf_;
    }
    SPECULA_CPU_GPU bool operator!=(const SampledWavelengths &swl) const {
      return lambda != swl.lambda || pdf_ != swl.pdf_;
    }

    SPECULA_CPU_GPU [[nodiscard]] bool secondary_terminated() const {
      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        if (pdf_[i] != 0) {
          return false;
        }
      }
      return true;
    }

    SPECULA_CPU_GPU void terminate_secondary() {
      if (secondary_terminated()) {
        return;
      }

      for (int i = 1; i < N_SPECTRUM_SAMPLES; ++i) {
        pdf_[i] = 0;
      }
      pdf_[0] /= N_SPECTRUM_SAMPLES;
    }

  private:
    pstd::array<Float, N_SPECTRUM_SAMPLES> lambda, pdf_;

    friend struct fmt::formatter<SampledWavelengths>;
  };

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

  class BlackbodySpectrum {
  public:
    SPECULA_CPU_GPU BlackbodySpectrum(Float T) : T(T) {
      Float lambda_max = 2.8977721e-3f / T;
      normalization_factor = 1 / blackbody(lambda_max * 1e9f, T);
    }

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      return blackbody(lambda, T) * normalization_factor;
    }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return 1.0f; }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = blackbody(lambda[i], T) * normalization_factor;
      }
      return s;
    }

  private:
    Float T;
    Float normalization_factor;

    friend struct fmt::formatter<BlackbodySpectrum>;
  };

  class RgbAlbedoSpectrum {
  public:
    SPECULA_CPU_GPU RgbAlbedoSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU Float operator()(Float lambda) const { return rsp(lambda); }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return rsp.max_value(); }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = rsp(lambda[i]);
      }
      return s;
    }

  private:
    RgbSigmoidPolynomial rsp;

    friend struct fmt::formatter<RgbAlbedoSpectrum>;
  };

  class RgbUnboundedSpectrum {
  public:
    SPECULA_CPU_GPU RgbUnboundedSpectrum() : rsp(0, 0, 0), scale(0) {}
    SPECULA_CPU_GPU RgbUnboundedSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU Float operator()(Float lambda) const { return scale * rsp(lambda); }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const { return scale * rsp.max_value(); }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = scale * rsp(lambda[i]);
      }
      return s;
    }

  private:
    Float scale = 1;
    RgbSigmoidPolynomial rsp;

    friend struct fmt::formatter<RgbUnboundedSpectrum>;
  };

  class RgbIlluminantSpectrum {
  public:
    SPECULA_CPU_GPU RgbIlluminantSpectrum() = default;
    SPECULA_CPU_GPU RgbIlluminantSpectrum(const RgbColorSpace &cs, Rgb rgb);

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      if (illuminant == nullptr) {
        return 0.0f;
      }
      return scale * rsp(lambda) * (*illuminant)(lambda);
    }

    SPECULA_CPU_GPU [[nodiscard]] Float max_value() const {
      if (illuminant == nullptr) {
        return 0.0f;
      }
      return scale * rsp.max_value() * illuminant->max_value();
    }

    SPECULA_CPU_GPU [[nodiscard]] SampledSpectrum sample(const SampledWavelengths &lambda) const {
      if (illuminant == nullptr) {
        return SampledSpectrum(0);
      }

      SampledSpectrum s;
      for (int i = 0; i < N_SPECTRUM_SAMPLES; ++i) {
        s[i] = scale * rsp(lambda[i]);
      }
      return s * illuminant->sample(lambda);
    }

    const DenselySampledSpectrum *illuminant;

  private:
    Float scale;
    RgbSigmoidPolynomial rsp;

    friend struct fmt::formatter<RgbIlluminantSpectrum>;
  };

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

  SPECULA_CPU_GPU inline Float inner_product(Spectrum f, Spectrum g) {
    Float integral = 0;
    for (Float lambda = LAMBDA_MIN; lambda <= LAMBDA_MAX; ++lambda) {
      integral += f(lambda) * g(lambda);
    }
    return integral;
  }

  SPECULA_CPU_GPU inline Float blackbody(Float lambda, Float t) {
    if (t <= 0) {
      return 0;
    }

    const Float c = 299792458.f;
    const Float h = 6.62606957e-34f;
    const Float kb = 1.3806488e-23f;

    Float l = lambda * 1e-9f;
    Float le = (2 * h * c * c) / (pow<5>(l) * (fast_exp((h * c) / (l * kb * t)) - 1));
    ASSERT(!isnan(le));
    return le;
  }

  SPECULA_CPU_GPU inline Float sample_visible_wavelengths(Float u) {
    return 538 - 138.888889f * std::atanh(0.85691062f - 1.82750197f * u);
  }

  SPECULA_CPU_GPU inline Float visible_wavelengths_pdf(Float lambda) {
    if (lambda < 360 || lambda > 830) {
      return 0;
    }
    return 0.0039398042f / sqr(std::cosh(0.0072f * (lambda - 538)));
  }

  Float spectrum_to_photometric(Spectrum s);
  Xyz spectrum_to_xyz(Spectrum s);

  Spectrum get_named_spectrum(const std::string &name);
  std::string find_matching_named_spectrum(Spectrum s);

} // namespace specula

template <> struct std::hash<specula::DenselySampledSpectrum> {
  SPECULA_CPU_GPU size_t operator()(const specula::DenselySampledSpectrum &s) const {
    return specula::hash_buffer(s.values.data(), s.values.size());
  }
};

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

template <> struct fmt::formatter<specula::BlackbodySpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::BlackbodySpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ BlackbodySpectrum T={} ]", v.T);
  }
};

template <> struct fmt::formatter<specula::ConstantSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::ConstantSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ ConstantSpectrum c={} ]", v.c);
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

template <> struct fmt::formatter<specula::SampledSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::SampledSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "{}", fmt::join(v.values, ", "));
  }
};

template <> struct fmt::formatter<specula::SampledWavelengths> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::SampledWavelengths &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ SampledWavelengths lambda={} pdf={} ]",
                     fmt::join(v.lambda, ", "), fmt::join(v.pdf_, ", "));
  }
};

template <> struct fmt::formatter<specula::RgbAlbedoSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RgbAlbedoSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ RgbAlbedoSpectrum rsp={} ]", v.rsp);
  }
};

template <> struct fmt::formatter<specula::RgbUnboundedSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RgbUnboundedSpectrum &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ RgbUnboundedSpectrum rsp={} ]", v.rsp);
  }
};

template <> struct fmt::formatter<specula::RgbIlluminantSpectrum> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::RgbIlluminantSpectrum &v, FormatContext &ctx) const {
    if (v.illuminant) {
      return format_to(ctx.out(), "[ RgbIlluminantSpectrum rsp={} scale={} illuminant={} ]", v.rsp,
                       v.scale, *v.illuminant);
    } else {
      return format_to(ctx.out(), "[ RgbIlluminantSpectrum rsp={} scale={} illuminant=(nullptr) ]",
                       v.rsp, v.scale);
    }
  }
};

#endif // SPECULA_UTIL_SPECTRUM_HPP
