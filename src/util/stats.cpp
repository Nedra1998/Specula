#include "specula/util/stats.hpp"

#include <any>
#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>

#include <fmt/ostream.h>
#include <tracy/Tracy.hpp>

#include "specula/types.hpp"
#include "specula/util/check.hpp"
#include "specula/util/image/image.hpp"
#include "specula/util/memory.hpp"
#include "specula/util/string.hpp"
#include "specula/util/vecmath.hpp"

static void get_category_and_title(const std::string &str, std::string *category,
                                   std::string *title) {
  std::vector<std::string> comps = specula::split_string(str, '/');
  if (comps.size() == 1) {
    *title = comps[0];
  } else {
    ASSERT_EQ(comps.size(), 2);
    *category = comps[0];
    *title = comps[1];
  }
}

namespace specula {
  struct ThreadStatsState {
    Point2i p;
    bool active = false;
    std::chrono::steady_clock::time_point start;
    PixelStatsAccumulator accum;
  };

  bool pixel_stats_enabled = false;
  static thread_local ThreadStatsState thread_stats_state;

  static std::vector<StatRegisterer::AccumFunc> stat_funcs;
  static std::vector<StatRegisterer::PixelAccumFunc> pixel_stat_funcs;

  static StatsAccumulator stats_accumulator;

  static Bounds2i image_bounds;
  std::string pixel_stats_base_name;

  struct PixelStatsAccumulator::PixelStats {
    Image time;
    std::vector<std::string> counter_names;
    std::vector<Image> counter_images;
    std::vector<std::string> ratio_names;
    std::vector<Image> ratio_images;
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

    Image pixel_time;
    std::vector<std::string> pixel_counter_names;
    std::vector<Image> pixel_counter_images;
    std::vector<std::string> pixel_ratio_names;
    std::vector<Image> pixel_ratio_images;
  };
} // namespace specula

specula::StatRegisterer::StatRegisterer(AccumFunc func, PixelAccumFunc pfunc) {
  static TracyLockable(std::mutex, mutex);
  std::lock_guard<std::mutex> lock(mutex);
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

void specula::StatRegisterer::call_pixel_callbacks(Point2i p, PixelStatsAccumulator &accum) {
  for (std::size_t i = 0; i < pixel_stat_funcs.size(); ++i) {
    pixel_stat_funcs[i](p, i, accum);
  }
}

specula::PixelStatsAccumulator::PixelStatsAccumulator() : stats(new PixelStats) {}

void specula::PixelStatsAccumulator::report_pixel_ms(Point2i p, float ms) {
  auto res = Point2i(image_bounds.diagonal());
  if (stats->time.resolution() != res) {
    stats->time = Image(PixelFormat::FLOAT, res, {"ms"});
  }

  auto pp = Point2i(p - image_bounds.p_min);
  stats->time.set_channel(pp, 0, stats->time.get_channel(pp, 0) + ms);
}

void specula::PixelStatsAccumulator::report_counter(Point2i p, int counter_index, const char *name,
                                                    int64_t val) {
  if (counter_index >= stats->counter_images.size()) {
    stats->counter_images.resize(counter_index + 1);
    stats->counter_names.resize(counter_index + 1);
    stats->counter_names[counter_index] = name;
  }

  Image &img = stats->counter_images[counter_index];
  auto res = Point2i(image_bounds.diagonal());
  if (img.resolution() != res) {
    img = Image(PixelFormat::FLOAT, res, {"count"});
  }

  auto pp = Point2i(p - image_bounds.p_min);
  img.set_channel(pp, 0, img.get_channel(pp, 0) + val);
}

void specula::PixelStatsAccumulator::report_ratio(Point2i p, int counter_index, const char *name,
                                                  int64_t num, int64_t denom) {

  if (counter_index >= stats->ratio_images.size()) {
    stats->ratio_images.resize(counter_index + 1);
    stats->ratio_names.resize(counter_index + 1);
    stats->ratio_names[counter_index] = name;
  }

  Image &img = stats->ratio_images[counter_index];
  auto res = Point2i(image_bounds.diagonal());
  if (img.resolution() != res) {
    img = Image(PixelFormat::FLOAT, res, {"numerator", "denominator", "ratio"});
  }

  auto pp = Point2i(p - image_bounds.p_min);
  img.set_channel(pp, 0, img.get_channel(pp, 0) + num);
  img.set_channel(pp, 1, img.get_channel(pp, 1) + denom);
  if (img.get_channel(pp, 0) == 0) {
    img.set_channel(pp, 2, 0);
  } else {
    img.set_channel(pp, 2, img.get_channel(pp, 0) / img.get_channel(pp, 1));
  }
}

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

void specula::StatsAccumulator::accumulate_pixel_stats(const PixelStatsAccumulator &accum) {
  auto res = Point2i(image_bounds.diagonal());
  if (stats->pixel_time.resolution() == Point2i(0, 0)) {
    stats->pixel_time = Image(PixelFormat::FLOAT, res, {"ms"});
  } else {
    ASSERT_EQ(stats->pixel_time.resolution(), res);
  }
  ASSERT_EQ(stats->pixel_time.resolution(), accum.stats->time.resolution());

  for (int y = 0; y < stats->pixel_time.resolution().y; ++y) {
    for (int x = 0; x < stats->pixel_time.resolution().x; ++x) {
      stats->pixel_time.set_channel(
          {x, y}, 0,
          (stats->pixel_time.get_channel({x, y}, 0) + accum.stats->time.get_channel({x, y}, 0)));
    }
  }

  if (stats->pixel_counter_images.size() < accum.stats->counter_images.size()) {
    stats->pixel_counter_images.resize(accum.stats->counter_images.size());
    stats->pixel_counter_names.resize(accum.stats->counter_names.size());
  }

  if (stats->pixel_ratio_images.size() < accum.stats->ratio_images.size()) {
    stats->pixel_ratio_images.resize(accum.stats->ratio_images.size());
    stats->pixel_ratio_names.resize(accum.stats->ratio_names.size());
  }

  for (size_t i = 0; i < accum.stats->counter_images.size(); ++i) {
    if (stats->pixel_counter_names[i].empty()) {
      stats->pixel_counter_names[i] = accum.stats->counter_names[i];
    }

    const Image &thread_image = accum.stats->counter_images[i];
    if (thread_image.resolution() == Point2i(0, 0)) {
      continue;
    }
    Image &accum_image = stats->pixel_counter_images[i];
    if (accum_image.resolution() == Point2i(0, 0)) {
      accum_image = Image(PixelFormat::FLOAT, thread_image.resolution(), {"count"});
    }

    for (int y = 0; y < thread_image.resolution().y; ++y) {
      for (int x = 0; x < thread_image.resolution().x; ++x) {
        accum_image.set_channel(
            {x, y}, 0, (accum_image.get_channel({x, y}, 0) + thread_image.get_channel({x, y}, 0)));
      }
    }
  }

  for (size_t i = 0; i < accum.stats->ratio_images.size(); ++i) {
    if (stats->pixel_ratio_names[i].empty()) {
      stats->pixel_ratio_names[i] = accum.stats->ratio_names[i];
    }

    const Image &thread_image = accum.stats->ratio_images[i];
    if (thread_image.resolution() == Point2i(0, 0)) {
      continue;
    }
    Image &accum_image = stats->pixel_ratio_images[i];
    if (accum_image.resolution() == Point2i(0, 0)) {
      accum_image = Image(PixelFormat::FLOAT, thread_image.resolution(),
                          {"numerator", "denominator", "ratio"});
    }

    for (int y = 0; y < thread_image.resolution().y; ++y) {
      for (int x = 0; x < thread_image.resolution().x; ++x) {
        accum_image.set_channel(
            {x, y}, 0, (accum_image.get_channel({x, y}, 0) + thread_image.get_channel({x, y}, 0)));
        accum_image.set_channel(
            {x, y}, 1, (accum_image.get_channel({x, y}, 1) + thread_image.get_channel({x, y}, 1)));

        if (accum_image.get_channel({x, y}, 0) == 0) {
          accum_image.set_channel({x, y}, 2, 0.0f);
        } else {
          accum_image.set_channel(
              {x, y}, 2, (accum_image.get_channel({x, y}, 0) / accum_image.get_channel({x, y}, 1)));
        }
      }
    }
  }
}

void specula::StatsAccumulator::write_pixel_images() const {
  ASSERT(stats->pixel_time.write(pixel_stats_base_name + "-time.exr"));

  auto rewrite_slashes = [](std::string s) {
    for (char &i : s) {
      if (i == '/') {
        i = '_';
      }
    }
    return s;
  };

  auto all_zero = [](const Image &img) {
    for (int y = 0; y < img.resolution().y; ++y) {
      for (int x = 0; x < img.resolution().x; ++x) {
        if (img.get_channels({x, y}).max_value() != 0) {
          return false;
        }
      }
    }
    return true;
  };

  for (size_t i = 0; i < stats->pixel_counter_images.size(); ++i) {
    std::string name =
        pixel_stats_base_name + "-" + rewrite_slashes(stats->pixel_counter_names[i]) + ".exr";
    if (!all_zero(stats->pixel_counter_images[i])) {
      ASSERT(stats->pixel_counter_images[i].write(name));
    }
  }

  for (size_t i = 0; i < stats->pixel_ratio_images.size(); ++i) {
    std::string name =
        pixel_stats_base_name + "-" + rewrite_slashes(stats->pixel_ratio_names[i]) + ".exr";
    if (!all_zero(stats->pixel_ratio_images[i])) {
      ASSERT(stats->pixel_ratio_images[i].write(name));
    }
  }
}

void specula::StatsAccumulator::print(std::ostream &dest) {
  fmt::println(dest, "Statistics:");
  std::map<std::string, std::vector<std::string>> to_print;

  for (auto &counter : stats->counters) {
    if (counter.second == 0) {
      continue;
    }
    std::string category, title;
    get_category_and_title(counter.first, &category, &title);
    to_print[category].push_back(fmt::format("{:42} {:12}", title, counter.second));
  }

  size_t total_memory_reported = 0;
  auto print_bytes = [](size_t bytes) -> std::string {
    float kb = static_cast<double>(bytes) / 1024.0;
    if (std::abs(kb) < 1024.0) {
      return fmt::format("{:9.2f} kB", kb);
    }
    float mib = kb / 1024.0;
    if (std::abs(mib) < 1024.0) {
      return fmt::format("{:9.2f} MiB", mib);
    }
    float gib = mib / 1024.0;
    return fmt::format("{:9.2f} GiB", gib);
  };

  for (auto &counter : stats->memory_counters) {
    if (counter.second == 0) {
      continue;
    }
    total_memory_reported += counter.second;

    std::string category, title;
    get_category_and_title(counter.first, &category, &title);
    to_print[category].push_back(fmt::format("{:42} {}", title, print_bytes(counter.second)));
  }

  int64_t unreported_bytes = get_current_rss() - total_memory_reported;
  if (unreported_bytes > 0) {
    to_print["Memory"].push_back(
        fmt::format("{:42} {}", "Unreported / Unused", print_bytes(unreported_bytes)));
  }

  for (auto &distrib : stats->int_distributions) {
    if (distrib.second.count == 0) {
      continue;
    }
    std::string category, title;
    get_category_and_title(distrib.first, &category, &title);
    double avg =
        static_cast<double>(distrib.second.sum) / static_cast<double>(distrib.second.count);
    to_print[category].push_back(fmt::format("{:42} {:.3} avg [range {} - {}]", title, avg,
                                             distrib.second.min, distrib.second.max));
  }

  for (auto &distrib : stats->float_distributions) {
    if (distrib.second.count == 0) {
      continue;
    }
    std::string category, title;
    get_category_and_title(distrib.first, &category, &title);
    double avg =
        static_cast<double>(distrib.second.sum) / static_cast<double>(distrib.second.count);
    to_print[category].push_back(fmt::format("{:42} {:.3} avg [range {} - {}]", title, avg,
                                             distrib.second.min, distrib.second.max));
  }

  for (auto &percentage : stats->percentages) {
    if (percentage.second.second == 0) {
      continue;
    }
    int64_t num = percentage.second.first;
    int64_t denom = percentage.second.second;
    std::string category, title;
    get_category_and_title(percentage.first, &category, &title);
    to_print[category].push_back(
        fmt::format("{:42} {} / {} ({:.2f}%)", title, num, denom, (100.0f * num) / denom));
  }

  for (auto &ratio : stats->ratios) {
    if (ratio.second.second == 0) {
      continue;
    }
    int64_t num = ratio.second.first;
    int64_t denom = ratio.second.second;
    std::string category, title;
    get_category_and_title(ratio.first, &category, &title);
    to_print[category].push_back(
        fmt::format("{:42} {} / {} ({:.2f}x)", title, num, denom,
                    static_cast<double>(num) / static_cast<double>(denom)));
  }

  for (auto &categories : to_print) {
    fmt::println(dest, "  {}", categories.first.c_str());
    for (auto &item : categories.second) {
      fmt::println(dest, "    {}", item.c_str());
    }
  }
}

bool specula::StatsAccumulator::print_check_rare(std::ostream &dest) {
  bool any_failed = false;

  for (const auto &iter : stats->rare_checks) {
    const Stats::RareCheck &rc = iter.second;
    Float true_freq = static_cast<double>(rc.num_true) / static_cast<double>(rc.total);
    Float variance_estimate = 1 / static_cast<double>(rc.total - 1) * true_freq * (1 - true_freq);
    if (true_freq - 2 * variance_estimate >= rc.max_frequency) {
      fmt::println(dest,
                   "{} @ {:.9} failures was {}x over limit {:.9} ({} samples, sigma est {:.9}",
                   iter.first, true_freq, true_freq / rc.max_frequency, rc.max_frequency, rc.total,
                   variance_estimate);
      any_failed = true;
    }
  }
  return any_failed;
}

void specula::StatsAccumulator::clear() {
  stats->counters.clear();
  stats->memory_counters.clear();
  stats->int_distributions.clear();
  stats->float_distributions.clear();
  stats->percentages.clear();
  stats->ratios.clear();
}

void specula::stats_enable_pixel_stats(const Bounds2i &b, const std::string &base_name) {
  pixel_stats_enabled = true;
  image_bounds = b;
  pixel_stats_base_name = base_name;
}

void specula::stats_report_pixel_start(const Point2i &p) {
  if (!pixel_stats_enabled) {
    return;
  }
  ASSERT(thread_stats_state.active == false);
  thread_stats_state.active = true;
  thread_stats_state.p = p;
  thread_stats_state.start = std::chrono::steady_clock::now();
}

void specula::stats_report_pixel_end(const Point2i &p) {
  if (!pixel_stats_enabled) {
    return;
  }

  ThreadStatsState &tss = thread_stats_state;
  ASSERT(tss.active == true && tss.p == p);
  tss.active = false;

  auto elapsed = std::chrono::steady_clock::now() - tss.start;
  float delta_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() / 1e6f;
  tss.accum.report_pixel_ms(p, delta_ms);

  StatRegisterer::call_pixel_callbacks(p, tss.accum);
}

void specula::print_stats(std::ostream &dest) { stats_accumulator.print(dest); }
void specula::stats_write_pixel_images() { stats_accumulator.write_pixel_images(); }
bool specula::print_check_rare(std::ostream &dest) {
  return stats_accumulator.print_check_rare(dest);
}
void specula::clear_stats() { stats_accumulator.clear(); }

void specula::report_thread_stats() {
  static TracyLockable(std::mutex, mutex);
  std::lock_guard<std::mutex> lock(mutex);
  StatRegisterer::call_callbacks(stats_accumulator);
  if (pixel_stats_enabled) {
    stats_accumulator.accumulate_pixel_stats(thread_stats_state.accum);
    thread_stats_state.accum = PixelStatsAccumulator();
  }
}
