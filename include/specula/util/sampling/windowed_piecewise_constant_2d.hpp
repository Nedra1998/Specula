#ifndef SPECULA_UTIL_SAMPLING_WINDOWED_PIECEWISE_CONSTANT_2D_HPP
#define SPECULA_UTIL_SAMPLING_WINDOWED_PIECEWISE_CONSTANT_2D_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/containers.hpp"
#include "specula/util/sampling/summed_area_table.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  class WindowedPiecewiseConstant2D {
  public:
    WindowedPiecewiseConstant2D(Allocator alloc) : sat(alloc), func(alloc) {}
    WindowedPiecewiseConstant2D(const Array2D<Float> &f, Allocator alloc = {})
        : sat(f, alloc), func(f, alloc) {}

    SPECULA_CPU_GPU pstd::optional<Point2f> sample(Point2f u, Bounds2f b, Float *pdf) const {
      if (sat.integral(b) == 0) {
        return {};
      }

      Float bint = sat.integral(b);
      auto px = [&, this](Float x) -> Float {
        Bounds2f bx = b;
        bx.p_max.x = x;
        return sat.integral(bx) / bint;
      };

      Point2f p;
      p.x = sample_bisection(px, u[0], b.p_min.x, b.p_max.x, func.xsize());

      int nx = func.xsize();
      Bounds2f bcond(Point2f(pstd::floor(p.x * nx) / nx, b.p_min.y),
                     Point2f(pstd::ceil(p.x * nx) / nx, b.p_max.y));
      if (bcond.p_min.x == bcond.p_max.x) {
        bcond.p_max.x += 1.0f / nx;
      }
      if (sat.integral(bcond) == 0) {
        return {};
      }

      Float cond_int = sat.integral(bcond);

      auto py = [&, this](Float y) -> Float {
        Bounds2f by = bcond;
        by.p_max.y = y;
        return sat.integral(by) / cond_int;
      };
      p.y = sample_bisection(py, u[1], b.p_min.y, b.p_max.y, func.ysize());
      *pdf = eval(p) / bint;
      return p;
    }

    SPECULA_CPU_GPU [[nodiscard]] Float pdf(Point2f p, const Bounds2f &b) const {
      Float func_int = sat.integral(b);
      if (func_int == 0) {
        return 0;
      }
      return eval(p) / func_int;
    }

  private:
    template <typename Cdf>
    SPECULA_CPU_GPU static Float sample_bisection(Cdf pred, Float u, Float min, Float max, int n) {
      while (pstd::ceil(n * max) - pstd::floor(n * min) > 1) {
        DASSERT_LE(pred(min), u);
        DASSERT_GE(pred(max), u);
        Float mid = (min + max) / 2;
        if (pred(mid) > u) {
          max = mid;
        } else {
          min = mid;
        }
      }

      Float t = (u - pred(min)) / (pred(max) - pred(min));
      return clamp(lerp(t, min, max), min, max);
    }

    SPECULA_CPU_GPU [[nodiscard]] Float eval(Point2f p) const {
      Point2i pi(std::min<int>(p[0] * func.xsize(), func.xsize() - 1),
                 std::min<int>(p[1] * func.ysize(), func.ysize() - 1));
      return func[pi];
    }

    SummedAreaTable sat;
    Array2D<Float> func;
  };
} // namespace specula

#endif // SPECULA_UTIL_SAMPLING_WINDOWED_PIECEWISE_CONSTANT_2D_HPP
