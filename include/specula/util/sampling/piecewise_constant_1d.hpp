#ifndef SPECULA_UTIL_SAMPLING_PIECEWISE_CONSTANT_1D_HPP
#define SPECULA_UTIL_SAMPLING_PIECEWISE_CONSTANT_1D_HPP

// IWYU pragma: private, include "specula/util/sampling.hpp"

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/math.hpp"
#include "specula/util/pstd.hpp"

namespace specula {

  class PiecewiseConstant1D {
  public:
    PiecewiseConstant1D() = default;
    PiecewiseConstant1D(Allocator alloc) : func(alloc), cdf(alloc) {}
    PiecewiseConstant1D(pstd::span<const Float> f, Allocator alloc = {})
        : PiecewiseConstant1D(f, 0.0, 1.0, alloc) {}
    PiecewiseConstant1D(pstd::span<const Float> f, Float min, Float max, Allocator alloc = {})
        : func(f.begin(), f.end(), alloc), cdf(f.size() + 1, alloc), min(min), max(max) {
      ASSERT_GT(max, min);
      for (Float &f : func) {
        f = std::abs(f);
      }

      cdf[0] = 0;
      size_t n = f.size();
      for (size_t i = 1; i < n + 1; ++i) {
        ASSERT_GE(func[i - 1], 0);
        cdf[i] = cdf[i - 1] + func[i - 1] * (max - min) / n;
      }

      func_int = cdf[n];
      if (func_int == 0) {
        for (size_t i = 1; i < n + 1; ++i) {
          cdf[i] = Float(i) / Float(n);
        }
      } else {
        for (size_t i = 1; i < n + 1; ++i) {
          cdf[i] /= func_int;
        }
      }
    }

    SPECULA_CPU_GPU [[nodiscard]] size_t bytes_used() const {
      return (func.capacity() + cdf.capacity()) * sizeof(Float);
    }
    SPECULA_CPU_GPU [[nodiscard]] Float integral() const { return func_int; }
    SPECULA_CPU_GPU [[nodiscard]] size_t size() const { return func.size(); }

    SPECULA_CPU_GPU Float sample(Float u, Float *pdf = nullptr, int *offset = nullptr) const {
      size_t o = find_interval(cdf.size(), [&](int index) { return cdf[index] <= u; });
      if (offset != nullptr) {
        *offset = o;
      }

      Float du = u - cdf[o];
      if (cdf[o + 1] - cdf[o] > 0) {
        du /= cdf[o + 1] - cdf[o];
      }
      DASSERT(!isnan(du));

      if (pdf != nullptr) {
        *pdf = (func_int > 0) ? func[o] / func_int : 0;
      }

      return lerp((o + du) / size(), min, max);
    }

    SPECULA_CPU_GPU [[nodiscard]] pstd::optional<Float> invert(Float x) const {
      if (x < min || x > max) {
        return {};
      }

      Float c = (x - min) / (max - min) * func.size();
      int offset = clamp(c, 0, func.size() - 1);
      DASSERT(offset >= 0 && offset + 1 < cdf.size());

      Float delta = c - offset;
      return lerp(delta, cdf[offset], cdf[offset + 1]);
    }

    pstd::vector<Float> func, cdf;
    Float min{}, max{};
    Float func_int = 0;
  };
} // namespace specula

template <> struct fmt::formatter<specula::PiecewiseConstant1D> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::PiecewiseConstant1D &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ PiecewiseConstant1D func={} cdf={} min={} max={} funcInt={} ]",
                     v.func, v.cdf, v.min, v.max, v.func_int);
  }
};

#endif // SPECULA_UTIL_SAMPLING_PIECEWISE_CONSTANT_1D_HPP
