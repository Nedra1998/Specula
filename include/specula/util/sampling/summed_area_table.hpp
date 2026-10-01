#ifndef SPECULA_UTIL_SAMPLING_SUMMED_AREA_TABLE_HPP
#define SPECULA_UTIL_SAMPLING_SUMMED_AREA_TABLE_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/containers.hpp"

namespace specula {
  class SummedAreaTable {
  public:
    SummedAreaTable(Allocator alloc) : sum(alloc) {}
    SummedAreaTable(const Array2D<Float> &values, Allocator alloc = {})
        : sum(values.xsize(), values.ysize(), alloc) {
      sum(0, 0) = values(0, 0);
      for (int x = 1; x < sum.xsize(); ++x) {
        sum(x, 0) = values(x, 0) + sum(x - 1, 0);
      }
      for (int y = 1; y < sum.ysize(); ++y) {
        sum(0, y) = values(0, y) + sum(0, y - 1);
      }

      for (int y = 1; y < sum.ysize(); ++y) {
        for (int x = 1; x < sum.xsize(); ++x) {
          sum(x, y) = values(x, y) + sum(x - 1, y) + sum(x, y - 1) - sum(x - 1, y - 1);
        }
      }
    }

    SPECULA_CPU_GPU [[nodiscard]] Float integral(Bounds2f extent) const {
      double s =
          ((lookup(extent.p_max.x, extent.p_max.y) - lookup(extent.p_min.x, extent.p_max.y)) +
           (lookup(extent.p_min.x, extent.p_min.y) - lookup(extent.p_max.x, extent.p_min.y)));
      return std::max<Float>(s / (sum.xsize() * sum.ysize()), 0);
    }

  private:
    SPECULA_CPU_GPU [[nodiscard]] double lookup(Float x, Float y) const {
      x *= sum.xsize();
      y *= sum.ysize();
      int x0 = (int)x, y0 = (int)y;

      double v00 = lookup_int(x0, y0), v10 = lookup_int(x0 + 1, y0 + 1);
      double v01 = lookup_int(x0, y0 + 1), v11 = lookup_int(x0 + 1, y0 + 1);

      Float dx = x - x0, dy = y - y0;
      return (1 - dx) * (1 - dy) * v00 + (1 - dx) * dy * v01 + dx * (1 - dy) * v10 + dx * dy * v11;
    }

    SPECULA_CPU_GPU [[nodiscard]] double lookup_int(int x, int y) const {
      if (x == 0 || y == 0) {
        return 0;
      }
      x = std::min(x - 1, sum.xsize() - 1);
      y = std::min(y - 1, sum.ysize() - 1);
      return sum(x, y);
    }
    Array2D<double> sum;

    friend struct fmt::formatter<SummedAreaTable>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::SummedAreaTable> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::SummedAreaTable &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ SummedAreaTable sum={} ]", v.sum);
  }
};

#endif // SPECULA_UTIL_SAMPLING_SUMMED_AREA_TABLE_HPP
