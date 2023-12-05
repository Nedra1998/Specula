/**
 * @file stats.hpp
 * @brief Statistics collection and reporting
 * @date 2023-12-04
 * @copyright Copyright (c) 2023
 *
 * This file includes the interface used fo rcollecting statistics about the runtime behavior of the
 * system. It has been designed around being easy to add new measurements to track the system's
 * runtime behavior, and in consideration of the multi-threaded nature of renderer. And so to not
 * introduce any additional synchronization overhead, the statistics are collected and stored in
 * thread-local storage, and then aggregated when requested.
 */
#ifndef SPECULA_UTIL_STATS_HPP
#define SPECULA_UTIL_STATS_HPP

#include <cstdio>
#include <limits>
#include <string>

#include <specula/specula.hpp>

namespace specula {

/**
 * @brief Macro for registering a counter statistic
 *
 * A count of the number of instances of an event. The counter variable `var` can be updated as if
 * it was a regular integer variable; for example `++var`, `var += 10` are both valid.
 *
 d @param title The title of the statistic
 * @param var The variable name to use for the counter
 */
#define STAT_COUNTER(title, var)                                                                   \
  static thread_local int64_t var = 0;                                                             \
  static StatRegisterer STAT_REG##var([](StatsAccumulator &accum) {                                \
    accum.report_counter(title, var);                                                              \
    var = 0;                                                                                       \
  });

  class StatsAccumulator;

  /**
   * @brief Registerer for statistics collection
   *
   * This class is used to register a callback for collecting statistics about the system. The
   * callback is registered when the `StatRegisterer` is constructed, and then called when the
   * `StatsAccumulator` is asked to report the statistics.
   */
  class StatRegisterer {
  public:
    using AccumFunc = void (*)(StatsAccumulator &);

    /**
     * @brief Construct a new Stat Registerer object
     *
     * @param func The callback to register for collecting statistics
     */
    StatRegisterer(AccumFunc func);

    /**
     * @brief Call all registered callbacks
     *
     * @param accum The accumulator to pass to the callbacks for agregating the statistics
     */
    static void call_callbacks(StatsAccumulator &accum);
  };

  /**
   * @brief Accumulator for statistics
   *
   * This class is used to collect statistics about the system. It is designed to be used in
   * conjunction with the `StatRegisterer` class, which is used to register callbacks for collecting
   * the statistics.
   */
  class StatsAccumulator {
  public:
    StatsAccumulator();

    /**
     * @brief Report a counter value
     *
     * A count of the number of instances of an event.
     *
     * @param name The name of the statistic
     * @param val The value of the counter
     */
    void report_counter(const char *name, int64_t val);

    /**
     * @brief Report a memory counter value
     *
     * A specialized counter for recording memory usage. In particular, the values reported at the
     * end of rendering are in terms of kilobytes, megabytes, or gigabytes, as appropriate.
     *
     * @param name The name of the statistic
     * @param val The value of the counter
     */
    void report_memory_counter(const char *name, int64_t val);

    /**
     * @brief Report a percentage value
     *
     * Tracks how often a given event happens; the aggregate value is reported as the percentage of
     * `num/denom` when statistics are printed.
     *
     * @param name The name of the statistic
     * @param num The numerator of the percentage
     * @param denom The denominator of the percentage
     */
    void report_percentage(const char *name, int64_t num, int64_t denom);
    /**
     * @brief Report a ratio value
     *
     * This tracks how often an event happens but reports the result as a ration `num/denom` rather
     * than a percentage. This is often a more useful presentation if `num` is often greater than
     * `denom`.
     *
     * @param name The name of the statistic
     * @param num The numerator of the ratio
     * @param denom The denominator of the ratio
     */
    void report_ratio(const char *name, int64_t num, int64_t denom);
    /**
     * @brief Report a rare check from the `CHECK_RARE` macro
     *
     * This is used to report the frequency of occurrences of a condition that is allowed to happen,
     * as set by the `CHECK_RARE` macro.
     *
     * @param condition The condition that was checked
     * @param max_frequency The maximum allowed frequency that the condition should be true
     * @param num_true The number of times the condition was true
     * @param total The total number of times the condition was checked
     */
    void report_rare_check(const char *condition, Float max_frequency, int64_t num_true,
                           int64_t total);

  private:
    struct Stats;
    Stats *stats = nullptr;
  };
} // namespace specula

#endif // !SPECULA_UTIL_STATS_HPP
