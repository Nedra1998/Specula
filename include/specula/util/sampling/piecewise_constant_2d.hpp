#ifndef SPECULA_UTIL_SAMPLING_PIECEWISE_CONSTANT_2D_HPP
#define SPECULA_UTIL_SAMPLING_PIECEWISE_CONSTANT_2D_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/containers.hpp"
#include "specula/util/sampling/piecewise_constant_1d.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  class PiecewiseConstant2D {
  public:
    PiecewiseConstant2D() = default;
    PiecewiseConstant2D(Allocator alloc) : pconditionalv(alloc), pmarginal(alloc) {}
    PiecewiseConstant2D(pstd::span<const Float> data, int nx, int ny, Allocator alloc = {})
        : PiecewiseConstant2D(data, nx, ny, Bounds2f(Point2f(0, 0), Point2f(1, 1)), alloc) {}
    explicit PiecewiseConstant2D(const Array2D<Float> &data, Allocator alloc = {})
        : PiecewiseConstant2D(pstd::span<const Float>(data), data.xsize(), data.ysize(), alloc) {}
    PiecewiseConstant2D(const Array2D<Float> &data, Bounds2f domain, Allocator alloc = {})
        : PiecewiseConstant2D(pstd::span<const Float>(data), data.xsize(), data.ysize(), domain,
                              alloc) {}

    PiecewiseConstant2D(pstd::span<const Float> func, int nu, int nv, Bounds2f domain,
                        Allocator alloc = {})
        : domain_(domain), pconditionalv(alloc), pmarginal(alloc) {
      ASSERT_EQ(func.size(), nu * nv);
      pconditionalv.reserve(nv);
      for (size_t v = 0; v < nv; ++v) {
        pconditionalv.emplace_back(func.subspan(v * nu, nu), domain.p_min[0], domain.p_max[0],
                                   alloc);
      }

      pstd::vector<Float> marginal_func;
      marginal_func.reserve(nv);
      for (size_t v = 0; v < nv; ++v) {
        marginal_func.push_back(pconditionalv[v].integral());
      }

      pmarginal = PiecewiseConstant1D(marginal_func, domain.p_min[1], domain.p_max[1], alloc);
    }

    SPECULA_CPU_GPU [[nodiscard]] size_t bytes_used() const {
      return pconditionalv.size() * (pconditionalv[0].bytes_used() + sizeof(pconditionalv[0])) +
             pmarginal.bytes_used();
    }

    SPECULA_CPU_GPU [[nodiscard]] Bounds2f domain() const { return domain_; }
    SPECULA_CPU_GPU [[nodiscard]] Point2i resolution() const {
      return {int(pconditionalv[0].size()), int(pmarginal.size())};
    }

    SPECULA_CPU_GPU [[nodiscard]] Float integral() const { return pmarginal.integral(); }

    SPECULA_CPU_GPU Point2f sample(Point2f u, Float *pdf = nullptr,
                                   Point2i *offset = nullptr) const {
      Float pdfs[2];
      Point2i uv;
      Float d1 = pmarginal.sample(u[1], &pdfs[1], &uv[1]);
      Float d0 = pconditionalv[uv[1]].sample(u[0], &pdfs[0], &uv[0]);
      if (pdf != nullptr) {
        *pdf = pdfs[0] * pdfs[1];
      }
      if (offset != nullptr) {
        *offset = uv;
      }
      return {d0, d1};
    }

    SPECULA_CPU_GPU [[nodiscard]] Float pdf(Point2f pr) const {
      auto p = Point2f(domain_.offset(pr));
      int iu = clamp(p[0] * pconditionalv[0].size(), 0, pconditionalv[0].size() - 1);
      int iv = clamp(p[1] * pmarginal.size(), 0, pmarginal.size() - 1);
      return pconditionalv[iv].func[iu] / pmarginal.integral();
    }

    SPECULA_CPU_GPU [[nodiscard]] pstd::optional<Point2f> invert(Point2f p) const {
      pstd::optional<Float> minv = pmarginal.invert(p[1]);
      if (!minv) {
        return {};
      }

      Float p1o = (p[1] - domain_.p_min[1]) / (domain_.p_max[1] - domain_.p_min[1]);
      if (p1o < 0 || p1o > 1) {
        return {};
      }
      int offset = clamp(p1o * pconditionalv.size(), 0, pconditionalv.size() - 1);
      pstd::optional<Float> cinv = pconditionalv[offset].invert(p[0]);
      if (!cinv) {
        return {};
      }
      return Point2f(*cinv, *minv);
    }

  private:
    Bounds2f domain_;
    pstd::vector<PiecewiseConstant1D> pconditionalv;
    PiecewiseConstant1D pmarginal;

    friend struct fmt::formatter<PiecewiseConstant2D>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::PiecewiseConstant2D> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::PiecewiseConstant2D &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ PiecewiseConstant2D domain={} pConditionalV={} pMarginal={} ]",
                     v.domain_, v.pconditionalv, v.pmarginal);
  }
};

#endif // SPECULA_UTIL_SAMPLING_PIECEWISE_CONSTANT_2D_HPP
