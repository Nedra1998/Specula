#ifndef SPECULA_UTIL_MATH_SPLINE_HPP
#define SPECULA_UTIL_MATH_SPLINE_HPP

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/pstd/span.hpp"

namespace specula {
  SPECULA_CPU_GPU Float catmul_rom(pstd::span<const Float> nodes, pstd::span<const Float> values,
                                   Float x);
  SPECULA_CPU_GPU bool catmul_rom_weights(pstd::span<const Float> nodes, Float x, int *offset,
                                          pstd::span<Float> weights);
  SPECULA_CPU_GPU Float integrate_catmul_rom(pstd::span<const Float> nodes,
                                             pstd::span<const Float> values, pstd::span<Float> cdf);
  SPECULA_CPU_GPU Float invert_catmul_rom(pstd::span<const Float> nodes,
                                          pstd::span<const Float> values, Float u);
} // namespace specula

#endif // SPECULA_UTIL_MATH_SPLINE_HPP
