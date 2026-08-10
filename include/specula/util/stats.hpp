#ifndef SPECULA_UTIL_STATS_HPP
#define SPECULA_UTIL_STATS_HPP

#include <cstdint>
#include <limits>

#include "specula/types.hpp"

namespace specula {

#define STAT_COUNTER(title, var)                                                                   \
  static thread_local int64_t var;                                                                 \
  static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) {                               \
    accum.report_counter(title, var);                                                              \
    var = 0;                                                                                       \
  });

#define STAT_MEMORY_COUNTER(title, var)                                                            \
  static thread_local int64_t var;                                                                 \
  static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) {                               \
    accum.report_memory_counter(title, var);                                                       \
    var = 0;                                                                                       \
  });

#define STAT_INT_DISTRIBUTION(title, var)                                                          \
  static thread_local StatDistribution<int64_t> var;                                               \
  static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) {                               \
    accum.report_int_distribution(title, var.sum, var.count, var.min, var.max);                    \
    var.sum = 0;                                                                                   \
    var.count = 0;                                                                                 \
    var.min = std::numeric_limits<int64_t>::max();                                                 \
    var.max = std::numeric_limits<int64_t>::lowest();                                              \
  });

#define STAT_FLOAT_DISTRIBUTION(title, var)                                                        \
  static thread_local StatDistribution<double> var;                                                \
  static StatRegisterer STATS_REG##var([](StatsAccumulator &accum) {                               \
    accum.report_float_distribution(title, var.sum, var.count, var.min, var.max);                  \
    var.sum = 0;                                                                                   \
    var.count = 0;                                                                                 \
    var.min = std::numeric_limits<double>::max();                                                  \
    var.max = std::numeric_limits<double>::lowest();                                               \
  });

#define STAT_PERCENT(title, num_var, denom_var)                                                    \
  static thread_local int64_t num_var, denom_var;                                                  \
  static StatRegisterer STATS_REG##num_var([](StatsAccumulator &accum) {                           \
    accum.report_percentage(title, num_var, denom_var);                                            \
    num_var = 0;                                                                                   \
    denom_var = 0;                                                                                 \
  });

#define STAT_RATIO(title, num_var, denom_var)                                                      \
  static thread_local int64_t num_var, denom_var;                                                  \
  static StatRegisterer STATS_REG##num_var##denom_var([](StatsAccumulator &accum) {                \
    accum.report_ratio(title, num_var, denom_var);                                                 \
    num_var = 0;                                                                                   \
    denom_var = 0;                                                                                 \
  });

#define STAT_PIXEL_COUNTER(title, var)                                                             \
  static thread_local int64_t var, var##_sum;                                                      \
  static StatRegisterer STATS_REG##var(                                                            \
      [](StatsAccumulator &accum) {                                                                \
        accum.report_counter(title, var + var##_sum);                                              \
        var##_sum = 0;                                                                             \
        var = 0;                                                                                   \
      },                                                                                           \
      [](Point2i p, int counter_index, PixelStatsAccumulator &accum) {                             \
        accum.report_counter(p, counter_index, title, var);                                        \
        var##_sum += var;                                                                          \
        var = 0;                                                                                   \
      });

#define STAT_PIXEL_RATIO(title, num_var, denom_var)                                                \
  static thread_local int64_t num_var, num_var##_sum, denom_var, denom_var##_sum;                  \
  static StatRegisterer STATS_REG##num_var##denom_var(                                             \
      [](StatsAccumulator &accum) {                                                                \
        accum.report_ratio(title, num_var + num_var##_sum, denom_var + denom_var##_sum);           \
        num_var##_sum = 0;                                                                         \
        num_var = 0;                                                                               \
        denom_var##_sum = 0;                                                                       \
        denom_var = 0;                                                                             \
      },                                                                                           \
      [](Point2i p, int counter_index, PixelStatsAccumulator &accum) {                             \
        accum.report_ratio(p, counter_index, title, num_var, denom_var);                           \
        num_var##_sum += num_var;                                                                  \
        denom_var##_sum += denom_var;                                                              \
        num_var = 0;                                                                               \
        denom_var = 0;                                                                             \
      });

  class Point2i;

  class PixelStatsAccumulator {
  public:
    PixelStatsAccumulator();

    void report_pixel_ms(Point2i p, float ms);
    void report_counter(Point2i p, int counter_index, const char *name, int64_t val);
    void report_ratio(Point2i p, int counter_index, const char *name, int64_t num, int64_t denom);

  private:
    friend class StatsAccumulator;
    struct PixelStats;
    PixelStats *stats = nullptr;
  };

  class StatsAccumulator {
  public:
    StatsAccumulator();

    void report_counter(const char *name, int64_t val);
    void report_memory_counter(const char *name, int64_t val);
    void report_percentage(const char *name, int64_t num, int64_t denom);
    void report_ratio(const char *name, int64_t num, int64_t denom);
    void report_rare_check(const char *condition, Float max_frequency, int64_t num_true,
                           int64_t total);
    void report_int_distribution(const char *name, int64_t sum, int64_t count, int64_t min,
                                 int64_t max);
    void report_float_distribution(const char *name, double sum, int64_t count, double min,
                                   double max);

    void accumulate_pixel_stats(const PixelStatsAccumulator &accum);
    void write_pixel_images() const;

    void clear();

  private:
    struct Stats;
    Stats *stats = nullptr;
  };

  class StatRegisterer {
  public:
    using AccumFunc = void (*)(StatsAccumulator &);
    using PixelAccumFunc = void (*)(Point2i p, int counter_index, PixelStatsAccumulator &);

    StatRegisterer(AccumFunc func, PixelAccumFunc = {});

    static void call_callbacks(StatsAccumulator &accum);
    static void call_pixel_callbacks(Point2i p, PixelStatsAccumulator &accum);
  };

  template <typename T> struct StatDistribution {
    T sum = 0;
    T min = std::numeric_limits<T>::max();
    T max = std::numeric_limits<T>::lowest();

    int64_t count = 0;

    void operator<<(T value) {
      sum += value;
      ++count;
      min = (value < min) ? value : min;
      max = (value > max) ? value : max;
    }
  };

} // namespace specula

#endif // SPECULA_UTIL_STATS_HPP
