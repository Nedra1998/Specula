#include "util/spectrum/piecewise_linear_spectrum.hpp"

#include "util/check.hpp"
#include "util/math/functions.hpp"

specula::PiecewiseLinearSpectrum::PiecewiseLinearSpectrum(pstd::span<const Float> l,
                                                          pstd::span<const Float> v,
                                                          Allocator alloc)
    : lambdas(l.begin(), l.end(), alloc), values(v.begin(), v.end(), alloc) {
  ASSERT_EQ(lambdas.size(), values.size());
  for (size_t i = 0; i < lambdas.size() - 1; ++i) {
    ASSERT_LT(lambdas[i], lambdas[i + 1]);
  }
}

SPECULA_CPU_GPU specula::Float specula::PiecewiseLinearSpectrum::max_value() const {
  if (values.empty()) {
    return 0;
  }
  return *std::max_element(values.begin(), values.end());
}

SPECULA_CPU_GPU specula::Float specula::PiecewiseLinearSpectrum::operator()(Float lambda) const {
  if (lambdas.empty() || lambda < lambdas.front() || lambda > lambdas.back()) {
    return 0;
  }

  int o = find_interval(lambdas.size(), [&](int i) { return lambdas[i] <= lambda; });
  DASSERT(lambda >= lambdas[o] && lambda <= lambdas[o + 1]);
  Float t = (lambda - lambdas[o]) / (lambdas[o + 1] - lambdas[o]);
  return lerp(t, values[o], values[o + 1]);
}
