#ifndef SPECULA_UTIL_SAMPLING_WEIGHTED_RESERVOIR_SAMPLER_HPP
#define SPECULA_UTIL_SAMPLING_WEIGHTED_RESERVOIR_SAMPLER_HPP

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/rng.hpp"

namespace specula {
  template <typename T> class WeightedReservoirSampler {
  public:
    WeightedReservoirSampler() = default;
    SPECULA_CPU_GPU WeightedReservoirSampler(uint64_t rng_seed) : rng(rng_seed) {}

    SPECULA_CPU_GPU void seed(uint64_t seed) { rng.set_sequence(seed); }

    SPECULA_CPU_GPU bool add(const T &sample, Float weight) {
      weight_sum_ += weight;
      Float p = weight / weight_sum_;
      if (rng.uniform<Float>() < p) {
        reservoir = sample;
        reservoir_weight = weight;
        return true;
      }
      DASSERT_LT(weight_sum_, 1e80);
      return false;
    }

    template <typename F> SPECULA_CPU_GPU bool add(F func, Float weight) {
      weight_sum_ += weight;
      Float p = weight / weight_sum_;
      if (rng.uniform<Float>() < p) {
        reservoir = func();
        reservoir_weight = weight;
        return true;
      }
      DASSERT_LT(weight_sum_, 1e80);
      return false;
    }

    SPECULA_CPU_GPU void copy(const WeightedReservoirSampler &other) {
      weight_sum_ = other.weight_sum_;
      reservoir = other.reservoir;
      reservoir_weight = other.reservoir_weight;
    }

    SPECULA_CPU_GPU [[nodiscard]] bool has_sample() const { return weight_sum_ > 0; }
    SPECULA_CPU_GPU const T &get_sample() const { return reservoir; }
    SPECULA_CPU_GPU [[nodiscard]] Float sample_probability() const {
      return reservoir_weight / weight_sum_;
    }
    SPECULA_CPU_GPU [[nodiscard]] Float weight_sum() const { return weight_sum_; }

    SPECULA_CPU_GPU void reset() { reservoir_weight = weight_sum_ = 0; }

    SPECULA_CPU_GPU void merge(const WeightedReservoirSampler &other) {
      DASSERT_LT(weight_sum_ + other.weight_sum_, 1e80);
      if (other.has_sample() && add(other.reservoir, other.weight_sum_)) {
        reservoir_weight = other.reservoir_weight;
      }
    }

  private:
    Rng rng;
    Float weight_sum_ = 0;
    Float reservoir_weight = 0;
    T reservoir{};

    friend struct fmt::formatter<WeightedReservoirSampler>;
  };
} // namespace specula

template <typename T> struct fmt::formatter<specula::WeightedReservoirSampler<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::WeightedReservoirSampler<T> &v, FormatContext &ctx) const {
    return format_to(
        ctx.out(),
        "[ WeightedReservoirSampler rng={} weightSum={} reservoir={} reservoirWeight={} ]", v.rng,
        v.weight_sum_, v.reservoir, v.reservoir_weight);
  }
};

#endif // SPECULA_UTIL_SAMPLING_WEIGHTED_RESERVOIR_SAMPLER_HPP
