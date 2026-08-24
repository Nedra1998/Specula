#include "specula/util/math/spline.hpp"

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/math/functions.hpp"

SPECULA_CPU_GPU specula::Float specula::catmul_rom(pstd::span<const Float> nodes,
                                                   pstd::span<const Float> values, Float x) {
  ASSERT_EQ(nodes.size(), values.size());
  if (x < nodes.front() || x > nodes.back()) {
    return 0;
  }
  size_t idx = find_interval(nodes.size(), [&](int i) { return nodes[i] <= x; });
  Float x0 = nodes[idx], x1 = nodes[idx + 1];
  Float f0 = values[idx], f1 = values[idx + 1];
  Float width = x1 - x0;
  Float d0 = NAN, d1 = NAN;
  if (idx > 0) {
    d0 = width * (f1 - values[idx - 1]) / (x1 - nodes[idx - 1]);
  } else {
    d0 = f1 - f0;
  }

  if (idx + 2 < nodes.size()) {
    d1 = width * (values[idx + 2] - f0) / (nodes[idx + 2] - x0);
  } else {
    d1 = f1 - f0;
  }

  Float t = (x - x0) / (x1 - x0), t2 = t * t, t3 = t2 * t;
  return (2 * t3 - 3 * t2 + 1) * f0 + (-2 * t3 + 3 * t2) * f1 + (t3 - 2 * t2 + t) * d0 +
         (t3 - t2) * d1;
}

SPECULA_CPU_GPU bool specula::catmul_rom_weights(pstd::span<const Float> nodes, Float x,
                                                 int *offset, pstd::span<Float> weights) {
  ASSERT_GE(weights.size(), 4);
  if (x < nodes.front() || x > nodes.back()) {
    return false;
  }

  size_t idx = find_interval(nodes.size(), [&](int i) { return nodes[i] <= x; });
  *offset = static_cast<int>(idx) - 1;

  Float x0 = nodes[idx], x1 = nodes[idx + 1];
  Float t = (x - x0) / (x1 - x0), t2 = t * t, t3 = t2 * t;

  weights[1] = 2 * t3 - 3 * t2 + 1;
  weights[2] = -2 * t3 + 3 * t2;

  if (idx > 0) {
    Float w0 = (t3 - 2 * t2 + t) * (x1 - x0) / (x1 - nodes[idx - 1]);
    weights[0] = -w0;
    weights[2] += w0;
  } else {
    Float w0 = t3 - 2 * t2 + t;
    weights[0] = 0;
    weights[1] -= w0;
    weights[2] += w0;
  }

  if (idx + 2 < nodes.size()) {
    Float w3 = (t3 - t2) * (x1 - x0) / (nodes[idx + 2] - x0);
    weights[1] -= w3;
    weights[3] = w3;
  } else {
    Float w3 = t3 - t2;
    weights[1] -= w3;
    weights[2] += w3;
    weights[3] = 0;
  }

  return true;
}

SPECULA_CPU_GPU specula::Float specula::integrate_catmul_rom(pstd::span<const Float> nodes,
                                                             pstd::span<const Float> values,
                                                             pstd::span<Float> cdf) {
  ASSERT_EQ(nodes.size(), values.size());
  Float sum = 0;

  cdf[0] = 0;
  for (size_t i = 0; i < nodes.size() - 1; ++i) {
    Float x0 = nodes[i], x1 = nodes[i + 1];
    Float f0 = values[i], f1 = values[i + 1];
    Float width = x1 - x0;

    Float d0 = (i > 0) ? width * (f1 - values[i - 1]) / (x1 - nodes[i - 1]) : (f1 - f0);
    Float d1 =
        (i + 2 < nodes.size()) ? width * (values[i + 2] - f0) / (nodes[i + 2] - x0) : (f1 - f0);

    sum += width * ((f0 + f1) / 2 + (d0 - d1) / 12);
    cdf[i + 1] = sum;
  }
  return sum;
}

SPECULA_CPU_GPU specula::Float specula::invert_catmul_rom(pstd::span<const Float> nodes,
                                                          pstd::span<const Float> values, Float u) {
  if (!(u > values.front())) {
    return nodes.front();
  } else if (!(u < values.back())) {
    return nodes.back();
  }

  size_t i = find_interval(values.size(), [&](size_t i) { return values[i] <= u; });

  Float x0 = nodes[i], x1 = nodes[i + 1];
  Float f0 = values[i], f1 = values[i + 1];
  Float width = x1 - x0;

  Float d0 = (i > 0) ? width * (f1 - values[i - 1]) / (x1 - nodes[i - 1]) : (f1 - f0);
  Float d1 =
      (i + 2 < nodes.size()) ? width * (values[i + 2] - f0) / (nodes[i + 2] - x0) : (f1 - f0);

  auto eval = [&](Float t) -> std::pair<Float, Float> {
    Float t2 = t * t, t3 = t2 * t;

    Float Fhat = (2 * t3 - 3 * t2 + 1) * f0 + (-2 * t3 + 3 * t2) * f1 + (t3 - 2 * t2 + t) * d0 +
                 (t3 - t2) * d1;
    Float fhat = (6 * t2 - 6 * t) * f0 + (-6 * t2 + 6 * t) * f1 + (3 * t2 - 4 * t + 1) * d0 +
                 (3 * t2 - 2 * t) * d1;
    return {Fhat - u, fhat};
  };

  Float t = newton_bisection(0, 1, eval);
  return x0 + t * width;
}
