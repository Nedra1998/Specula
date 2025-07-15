/**
 * @file progress_reporter.hpp
 * @brief Progress reporter and time tracking utilities.
 *
 * This file contains the implementation of a progress reporter and a timer class, which can be used
 * for tracking the progress of a task and measuring elapsed time. It is used as a general interface
 * in the renderer for reporting the progress back to the end user.
 */
#ifndef SPECULA_UTIL_PROGRESS_REPORTER_HPP

#define SPECULA_UTIL_PROGRESS_REPORTER_HPP

#include <chrono>
#include <functional>
#include <string_view>

#include <fmt/format.h>

#include "specula/util/pstd/optional.hpp"

namespace specula {
  /**
   * @class Timer
   * @brief A simple timer class for measuring elapsed time.
   */
  class Timer {
  public:
    using clock = std::chrono::steady_clock;

    /**
     * @brief Start a new timer.
     */
    Timer() { start = clock::now(); }

    /**
     * @brief Get the elapsed time since the timer was started in seconds.
     *
     * @return A double representing the elapsed time in seconds.
     */
    inline double elapsed() const {
      return std::chrono::duration<double>(clock::now() - start).count();
    }

  private:
    clock::time_point start;
  };

  /**
   * @class ProgressReporter
   * @brief A class for reporting progress of a task.
   *
   * This class is used within the renderer for reporting progress. It is
   * configured with a callback function that can be used to display the actual
   * progress status.
   *
   * The progress reporter will start a new thread which will periodically display the current state
   * of progress. As the task takes longer to complete, the frequency of updates will be reduced to
   * minimize the overhead of the progress reporter.
   */
  class ProgressReporter {
  public:
    /**
     * @brief Create a new progress reporter.
     */
    ProgressReporter() : quiet(true) {}
    /**
     * @brief Create a new progress reporter.
     *
     * @param total_work The total amount of work to be done.
     * @param title The title of the progress bar.
     * @param quiet Whether to suppress output or not.
     * @param gpu If the progress reporter is used for GPU tasks.
     */
    ProgressReporter(std::int64_t total_work, std::string title, bool quiet, bool gpu = false);

    /**
     * @brief Destroy the progress reporter.
     */
    ~ProgressReporter();

    /**
     * @brief Set the display callback for the progress reporter.
     *
     * @param cb The callback used to display the progress bar.
     */
    inline void
    set_callback(std::function<void(std::string_view, std::int64_t, std::int64_t, float)> cb) {
      update_callback = cb;
    }

    /**
     * @brief Get the total elapsed time in seconds since the progress reporter was started.
     *
     * @return The elapsed time in seconds.
     */
    inline double elapsed_seconds() const { return finish_time ? *finish_time : timer.elapsed(); }

    /**
     * @brief Update the progress of the task.
     *
     * @param num The number of units of work done.
     */
    inline void update(std::int64_t num = 1) {
#ifdef SPECULA_BUILD_GPU_RENDERER
      if (gpu_events.size() > 0) {
        if (gpu_events_launched_offset + num < gpu_evnets.size()) {
          while (num-- > 0) {
          }
          CHECK_EQ(cudaEventRecord(gpu_events[gpu_events_launched_offset]), cudaSuccess);
          ++gpu_events_launched_offset;
        }
      }
      return;
#endif
      if (num == 0 || quiet)
        return;
      work_done += num;
    }

    /**
     * @brief Mark the progress as done and stop the progress reporter.
     */
    void done();

    /**
     * @brief A display callback for rendering a progress bar.
     *
     * This function can be used as a display callback for the progress reporter. It will render a
     * progress bar in the following format:
     *
     * ```
     * Test ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╸              75% [00:00:07<00:00:02]
     * ```
     *
     * The components of the progress bar are as follows:
     * - The title of the progress bar.
     * - The bar itself, which is a series of `━` characters and colors to indicate progress.
     * - The percentage of work done.
     * - The elapsed time in `HH:MM:SS` format.
     * - The estimated time remaining in `HH:MM:SS` format.
     *
     * @param title The title of the progress bar.
     * @param work_done The amount of work done so far.
     * @param total_work The total amount of work to be done.
     * @param elapsed The elapsed time in seconds.
     * @param color If the progress bar should be colored or not.
     */
    static std::string print_bar(std::string_view title, std::int64_t work_done,
                                 std::int64_t total_work, float elapsed, bool color = false);

  private:
    constexpr static const std::uint8_t BAR_WIDTH = 50;
    constexpr static const std::string_view BAR_CHARS[] = {"━", "╸", "╺", "━"};

    void run();

    std::int64_t total_work;
    std::string title;
    bool quiet;
    Timer timer;
    std::atomic<std::int64_t> work_done;
    std::atomic<bool> exit_thread;
    std::thread update_thread;
    pstd::optional<float> finish_time;
    std::function<void(std::string_view, std::int64_t, std::int64_t, float)> update_callback =
        [](std::string_view, std::int64_t, std::int64_t, float) {};

#ifdef SPECULA_BUILD_GPU_RENDERER
    std::vector<cudaEvent_t> gpu_events;
    std::atomic<std::size_t> gpu_events_launched_offset;
    int gpu_events_finished_offset;
#endif
  };
} // namespace specula

template <> struct fmt::formatter<specula::Timer> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Timer &t, FormatContext &ctx) const {
    return format_to(ctx.out(), "{}s", t.elapsed());
  }
};

#endif // SPECULA_UTIL_PROGRESS_REPORTER_HPP
