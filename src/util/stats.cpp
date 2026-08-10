#include "specula/util/stats.hpp"

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <utility>

#include <tracy/Tracy.hpp>

#include "specula/types.hpp"

namespace specula {
  struct ThreadStatsState {
    // Point2i p;
    bool active = false;
    std::chrono::steady_clock::time_point start;
    PixelStatsAccumulator accum;
  };

  bool pixel_stats_enabled = false;
  static thread_local ThreadStatsState thread_stats_state;

  static std::vector<StatRegisterer::AccumFunc> stat_funcs;
  static std::vector<StatRegisterer::PixelAccumFunc> pixel_stat_funcs;

  static StatsAccumulator stats_accumulator;

  // static Bound2i image_bounds;
  std::string pixel_stats_base_name;

  struct PixelStatsAccumulator::PixelStats {
    // Image time;
    // std::vector<std::string> counter_names;
    // std::vector<Image> counter_images;
    // std::vector<std::string> ratio_names;
    // std::vector<Image> ratio_images;
  };

  struct StatsAccumulator::Stats {
    struct RareCheck {
      RareCheck(Float f = 0) : max_frequency(f) {}
      Float max_frequency;
      int64_t num_true = 0, total = 0;
    };

    std::map<std::string, int64_t> counters;
    std::map<std::string, int64_t> memory_counters;
    std::map<std::string, StatDistribution<int64_t>> int_distributions;
    std::map<std::string, StatDistribution<double>> float_distributions;
    std::map<std::string, std::pair<int64_t, int64_t>> percentages;
    std::map<std::string, std::pair<int64_t, int64_t>> ratios;
    std::map<std::string, RareCheck> rare_checks;

    // Image pixel_time;
    // std::vector<std::string> pixel_counter_names;
    // std::vector<Image> pixel_counter_images;
    // std::vector<std::string> pixel_ratio_names;
    // std::vector<Image> pixel_ratio_images;
  };
} // namespace specula

specula::StatRegisterer::StatRegisterer(AccumFunc func, PixelAccumFunc pfunc) {
  static TracyLockable(std::mutex, mutex);
  std::lock_guard<std::mutex> lock(mutex);
  LockMark(mutex);
  if (func != nullptr) {
    stat_funcs.push_back(func);
  }

  if (pfunc != nullptr) {
    pixel_stat_funcs.push_back(pfunc);
  }
}

void specula::StatRegisterer::call_callbacks(StatsAccumulator &accum) {
  for (AccumFunc func : stat_funcs) {
    func(accum);
  }
}

// void specula::StatRegisterer::call_pixel_callbacks(Point2i p, PixelStatsAccumulator &accum) {
//   for (std::size_t i = 0; i < pixel_stat_funcs.size(); ++i) {
//     (pixel_stat_funcs[i])(p, i, accum);
//   }
// }

specula::PixelStatsAccumulator::PixelStatsAccumulator() : stats(new PixelStats) {}

// void specula::PixelStatsAccumulator::report_pixel_ms(Point2i p, float ms) {
//   Point2i res = Point2i(image_bounds.diagonal());
//   if (stats->time.resolution() != res) {
//     stats->time = Image(PixelFormat::Float, res, {"ms"});
//   }
//
//   Point2i pp = Point2i(p - image_bounds.pmin);
//   stats->time.set_channel(pp, 0, stats->time.get_channel(pp, 0) + ms);
// }
//
// void specula::PixelStatsAccumulator::report_counter(Point2i p, int counter_index, const char
// *name,
//                                                     int64_t val) {
//   if (counter_index >= stats->counter_images.size()) {
//     stats->counter_images.resize(counter_index + 1);
//     stats->counter_names.resize(counter_index + 1);
//     stats->counter_names[counter_index] = name;
//   }
//
//   Image &img = stats->counter_images[counter_index];
//   Point2i res = Point2i(image_bounds.diagonal());
//   if (img.resolution() != res) {
//     img = Image(PixelFormat::Float, res, {"count"});
//   }
//
//   Point2i pp = Point2i(p - image_bounds.pmin);
//   img.set_channel(pp, 0, im.get_channel(pp, 0) + val);
// }
//
// void specula::PixelStatsAccumulator::report_ratio(Point2i p, int counter_index, const char *name,
//                                                   int64_t num, int64_t denom) {
//
//   if (counter_index >= stats->ratio_images.size()) {
//     stats->ratio_images.resize(counter_index + 1);
//     stats->ratio_names.resize(counter_index + 1);
//     stats->ratio_names[counter_index] = name;
//   }
//
//   Image &img = stats->ratio_images[counter_index];
//   Point2i res = Point2i(image_bounds.diagonal());
//   if (img.resolution() != res) {
//     img = Image(PixelFormat::Float, res, {"numerator", "denominator", "ratio"});
//   }
//
//   Point2i pp = Point2i(p - image_bounds.pmin);
//   img.set_channel(pp, 0, im.get_channel(pp, 0) + num);
//   img.set_channel(pp, 1, im.get_channel(pp, 1) + denom);
//   if (img.get_channel(pp, 0) == 0) {
//     img.set_channel(pp, 2, 0);
//   } else {
//     img.set_channel(pp, 2, im.get_channel(pp, 0) / im.get_channel(pp, 1));
//   }
// }

specula::StatsAccumulator::StatsAccumulator() : stats(new Stats) {}

void specula::StatsAccumulator::report_counter(const char *name, int64_t val) {
  stats->counters[name] += val;
}

void specula::StatsAccumulator::report_memory_counter(const char *name, int64_t val) {
  stats->memory_counters[name] += val;
}

void specula::StatsAccumulator::report_percentage(const char *name, int64_t num, int64_t denom) {
  stats->percentages[name].first += num;
  stats->percentages[name].second += denom;
}

void specula::StatsAccumulator::report_ratio(const char *name, int64_t num, int64_t denom) {
  stats->ratios[name].first += num;
  stats->ratios[name].second += denom;
}

void specula::StatsAccumulator::report_rare_check(const char *condition, Float max_frequency,
                                                  int64_t num_true, int64_t total) {
  if (!stats->rare_checks.contains(condition)) {
    stats->rare_checks[condition] = Stats::RareCheck(max_frequency);
  }
  Stats::RareCheck &rc = stats->rare_checks[condition];
  rc.num_true += num_true;
  rc.total += total;
}

void specula::StatsAccumulator::report_int_distribution(const char *name, int64_t sum,
                                                        int64_t count, int64_t min, int64_t max) {
  StatDistribution<int64_t> &dist = stats->int_distributions[name];
  dist.sum += sum;
  dist.count += count;
  dist.min = std::min(dist.min, min);
  dist.max = std::min(dist.max, max);
}

void specula::StatsAccumulator::report_float_distribution(const char *name, double sum,
                                                          int64_t count, double min, double max) {
  StatDistribution<double> &dist = stats->float_distributions[name];
  dist.sum += sum;
  dist.count += count;
  dist.min = std::min(dist.min, min);
  dist.max = std::min(dist.max, max);
}

void specula::StatsAccumulator::clear() {
  stats->counters.clear();
  stats->memory_counters.clear();
  stats->int_distributions.clear();
  stats->float_distributions.clear();
  stats->percentages.clear();
  stats->ratios.clear();
}
