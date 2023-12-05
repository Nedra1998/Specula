#include "specula/util/stats.hpp"

#include <map>

#include <tracy/Tracy.hpp>

struct specula::StatsAccumulator::Stats {
  /// Structure for tracking integer or floating point distribution statistics
  template <typename T> struct Distribution {
    /// The number of samples in the distribution
    int64_t count = 0;
    /// The minimum value in the distribution
    T min = std::numeric_limits<T>::max();
    /// The maximum value in the distribution
    T max = std::numeric_limits<T>::lowest();
    /// The sum of all values in the distribution
    T sum = 0;
  };

  /// Structure for tracking frequency of rare check statistics
  struct RareCheck {
    RareCheck(Float f = 0) : max_frequency(f) {}
    /// The maximum permissible frequency of the check
    Float max_frequency;
    /// The number of times the condition was true
    int64_t num_true = 0;
    /// The total number of times the condition was checked
    int64_t total = 0;
  };

  /// Counter statistics to be accumulated
  std::map<std::string, int64_t> counters;
  /// Memory counter statistics to be accumulated
  std::map<std::string, int64_t> memory_counters;
  /// Integer distribution statistics to be accumulated
  std::map<std::string, Distribution<int64_t>> int_distributions;
  /// Floating point distribution statistics to be accumulated
  std::map<std::string, Distribution<double>> double_distributions;
  /// Percentage statistics to be accumulated
  std::map<std::string, std::pair<int64_t, int64_t>> percentages;
  /// Ratio statistics to be accumulated
  std::map<std::string, std::pair<int64_t, int64_t>> ratios;
  /// Rare check statistics to be accumulated
  std::map<std::string, RareCheck> rare_checks;
};

namespace specula {
  static std::vector<specula::StatRegisterer::AccumFunc> *stat_funcs = nullptr;
}

specula::StatRegisterer::StatRegisterer(StatRegisterer::AccumFunc func) {
  static TracyLockable(std::mutex, mutex);
  std::lock_guard<std::mutex> lock(mutex);

  if (!stat_funcs)
    stat_funcs = new std::vector<AccumFunc>;

  if (func)
    stat_funcs->push_back(func);
}

void specula::StatRegisterer::call_callbacks(StatsAccumulator &accum) {
  for (AccumFunc func : *stat_funcs)
    func(accum);
}

specula::StatsAccumulator::StatsAccumulator() { stats = new Stats; }

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
  stats->percentages[name].first += num;
  stats->percentages[name].second += denom;
}
void specula::StatsAccumulator::report_rare_check(const char *condition, Float max_frequency,
                                                  int64_t num_true, int64_t total) {
  if (stats->rare_checks.find(condition) == stats->rare_checks.end())
    stats->rare_checks[condition] = Stats::RareCheck(max_frequency);

  Stats::RareCheck &rc = stats->rare_checks[condition];
  rc.num_true += num_true;
  rc.total += total;
}
