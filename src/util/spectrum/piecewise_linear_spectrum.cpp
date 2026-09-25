#include "specula/util/spectrum/piecewise_linear_spectrum.hpp"

#include <algorithm>

#include "specula/macros.hpp"
#include "specula/util/file.hpp"

specula::pstd::optional<specula::Spectrum>
specula::PiecewiseLinearSpectrum::read(const std::string &filename, Allocator alloc) {
  std::vector<Float> values = read_float_file(filename);
  if (values.empty()) {
    LOG_WARN("Unabled to read spectrum file {}", filename);
    return {};
  }

  if (values.size() % 2 != 0) {
    LOG_WARN("Extra value found in spectrum file {}", filename);
    return {};
  }

  pstd::vector<Float> lambda, v;
  for (size_t i = 0; i < values.size() / 2; ++i) {
    if (i > 0 && values[2 * i] <= lambda.back()) {
      LOG_WARN("Spectrum file invalid {}: at {}'th entry, wavelengths aren't increasing {} >= {}",
               filename, i, lambda.back(), values[2 * i]);
      return {};
    }
    lambda.push_back(values[2 * i]);
    v.push_back(values[2 * i + 1]);
  }
  return Spectrum(alloc.new_object<PiecewiseLinearSpectrum>(lambda, v, alloc));
}

specula::PiecewiseLinearSpectrum *
specula::PiecewiseLinearSpectrum::from_interleaved(pstd::span<const Float> samples, bool normalize,
                                                   Allocator alloc) {
  ASSERT_EQ(0, samples.size() % 2);
  size_t n = samples.size() / 2;
  pstd::vector<Float> lambda, values;

  if (samples[0] > LAMBDA_MIN) {
    lambda.push_back(LAMBDA_MIN - 1);
    values.push_back(samples[1]);
  }
  for (size_t i = 0; i < n; ++i) {
    lambda.push_back(samples[2 * i]);
    values.push_back(samples[2 * i + 1]);
    if (i > 0) {
      ASSERT_GT(lambda.back(), lambda[lambda.size() - 2]);
    }
  }

  if (lambda.back() < LAMBDA_MAX) {
    lambda.push_back(LAMBDA_MAX + 1);
    values.push_back(values.back());
  }

  auto *spec = alloc.new_object<PiecewiseLinearSpectrum>(lambda, values, alloc);

  if (normalize) {
    spec->scale(CIE_Y_INTEGRAL / inner_product(spec, &spectra::Y()));
  }

  return spec;
}

specula::PiecewiseLinearSpectrum::PiecewiseLinearSpectrum(pstd::span<const Float> lambda,
                                                          pstd::span<const Float> values,
                                                          Allocator alloc)
    : lambdas(lambda.begin(), lambda.end(), alloc), values(values.begin(), values.end(), alloc) {
  ASSERT_EQ(lambdas.size(), values.size());
  for (size_t i = 0; i < lambdas.size() - 1; ++i) {
    ASSERT_LT(lambda[i], lambdas[i + 1]);
  }
}

SPECULA_CPU_GPU specula::Float specula::PiecewiseLinearSpectrum::operator()(Float lambda) const {
  if (lambdas.empty() || lambda < lambdas.front() || lambda > lambdas.back()) {
    return 0.0;
  }

  size_t o = find_interval(lambdas.size(), [&](int i) { return lambdas[i] <= lambda; });
  DASSERT(lambda >= lambdas[o] && lambda <= lambdas[o + 1]);
  Float t = (lambda - lambdas[o]) / (lambdas[o + 1] - lambdas[o]);
  return lerp(t, values[o], values[o + 1]);
}

SPECULA_CPU_GPU [[nodiscard]] specula::Float specula::PiecewiseLinearSpectrum::max_value() const {
  if (values.empty()) {
    return 0.0;
  }
  return *std::ranges::max_element(values);
}
