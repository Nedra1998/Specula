#ifndef SPECULA_UTIL_SAMPLING_VARIANCE_ESTIMATOR_HPP
#define SPECULA_UTIL_SAMPLING_VARIANCE_ESTIMATOR_HPP

// IWYU pragma: private, include "specula/util/sampling.hpp"

#include "specula/macros.hpp"
#include "specula/types.hpp"

namespace specula {
  template <typename Float = Float> class VarianceEstimator {
  public:
    SPECULA_CPU_GPU void add(Float x) {
      ++n;
      Float delta = x - mean_;
      mean_ += delta / n;
      Float delta2 = x - mean_;
      s += delta * delta2;
    }

    SPECULA_CPU_GPU Float mean() const { return mean_; }
    SPECULA_CPU_GPU Float variance() const { return (n > 1) ? s / (n - 1) : 0; }
    SPECULA_CPU_GPU [[nodiscard]] int64_t count() const { return n; }

    SPECULA_CPU_GPU Float relative_variance() const {
      return (n < 1 || mean_ == 0) ? 0 : variance() / mean();
    }

    SPECULA_CPU_GPU void merge(const VarianceEstimator &ve) {
      if (ve.n == 0) {
        return;
      }

      s = s + ve.s + sqr(ve.mean_ - mean_) * n * ve.n / (n + ve.n);
      mean_ = (n * mean_ + ve.n * ve.mean_) / (n + ve.n);
      n += ve.n;
    }

  private:
    Float mean_ = 0, s = 0;
    int64_t n = 0;
  };

} // namespace specula

#endif // SPECULA_UTIL_SAMPLING_VARIANCE_ESTIMATOR_HPP
