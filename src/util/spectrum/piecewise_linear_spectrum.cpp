#include "util/spectrum/piecewise_linear_spectrum.hpp"

#include "util/check.hpp"
#include "util/file.hpp"
#include "util/log.hpp"
#include "util/math/functions.hpp"
#include "util/spectrum/constants.hpp"
#include "util/spectrum/spectra.hpp"
#include "util/spectrum/spectrum.hpp"

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

specula::pstd::optional<specula::Spectrum>
specula::PiecewiseLinearSpectrum::read(const std::string &filename, Allocator alloc) {
  std::vector<Float> vals = read_float_file(filename);
  if (vals.empty()) {
    LOG_WARN("Unable to read spectrum file {}.", filename);
    return {};
  }

  if (vals.size() % 2 != 0) {
    LOG_WARN("Extra value found in spectrum file {}.", filename);
    return {};
  }

  pstd::vector<Float> lambda, v;
  for (size_t i = 0; i < vals.size() / 2; ++i) {
    if (i > 0 && vals[2 * i] <= lambda.back()) {
      LOG_WARN("Spectrum file invalid {}: at {}'th entry, wavelengths aren't increasing {} >= {}.",
               filename, int(i), lambda.back(), vals[2 * i]);
      return {};
    }
    lambda.push_back(vals[2 * i]);
    v.push_back(vals[2 * i + 1]);
  }
  return Spectrum(alloc.new_object<PiecewiseLinearSpectrum>(lambda, v, alloc));
}

specula::PiecewiseLinearSpectrum *
specula::PiecewiseLinearSpectrum::from_interleaved(pstd::span<const Float> samples, bool normalize,
                                                   Allocator alloc) {
  ASSERT_EQ(0, samples.size() % 2);
  int n = samples.size() / 2;
  pstd::vector<Float> lambda, v;

  if (samples[0] > LAMBDA_MIN) {
    lambda.push_back(LAMBDA_MIN - 1);
    v.push_back(samples[1]);
  }
  for (size_t i = 0; i < n; ++i) {
    lambda.push_back(samples[2 * i]);
    v.push_back(samples[2 * i + 1]);
    if (i > 0) {
      ASSERT_GE(lambda.back(), lambda[lambda.size() - 2]);
    }
  }

  if (lambda.back() < LAMBDA_MAX) {
    lambda.push_back(LAMBDA_MAX + 1);
    v.push_back(v.back());
  }

  PiecewiseLinearSpectrum *spec = alloc.new_object<PiecewiseLinearSpectrum>(lambda, v, alloc);
  if (normalize) {
    spec->scale(CIE_Y_INTEGRAL / inner_product(spec, &spectra::Y()));
  }

  return spec;
}
