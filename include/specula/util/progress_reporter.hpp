#ifndef SPECULA_UTIL_PROGRESS_REPORTER_HPP
#define SPECULA_UTIL_PROGRESS_REPORTER_HPP

#include <chrono>
#include <utility>

#include <fmt/base.h>
#include <fmt/chrono.h>

#include "specula/util/pstd.hpp"

namespace specula {

  class Timer {
  public:
    Timer() { start = clock::now(); }
    [[nodiscard]] inline auto elapsed() const { return clock::now() - start; }
    [[nodiscard]] inline double elapsed_seconds() const {
      int64_t elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed()).count();
      return elapsed_us / 1000000.0;
    }

  private:
    using clock = std::chrono::steady_clock;

    friend struct fmt::formatter<Timer>;

    clock::time_point start;
  };

  class ProgressReporter {
  public:
    ProgressReporter() : quiet(true) {}
    ProgressReporter(int64_t total_work, std::string title, bool quiet, bool gpu = false);

    ~ProgressReporter();

    inline void set_callback(std::function<void(std::string_view, int64_t, int64_t, float)> cb) {
      update_callback = std::move(cb);
    }

    inline void update(int64_t num = 1) {
#ifdef SPECULA_BUILD_GPU_RENDERER
      if (gpu_events.size() > 0) {
        if (gpu_events_launched_offset + num < gpu_evnets.size()) {
          while (num-- > 0) {
          }
          ASSERT_EQ(cudaEventRecord(gpu_events[gpu_events_launched_offset]), cudaSuccess);
          ++gpu_events_launched_offset;
        }
      }
      return;

#endif
      if (num == 0 || quiet) {
        return;
      }
      work_done += num;
    }

    [[nodiscard]] inline double elapsed_seconds() const {
      return finish_time ? *finish_time : timer.elapsed_seconds();
    }

    void done();
    void run();

  private:
    friend struct fmt::formatter<ProgressReporter>;

    int64_t total_work = 0;
    std::string title;
    bool quiet;
    Timer timer;
    std::atomic<int64_t> work_done;
    std::atomic<bool> exit_thread;
    std::thread update_thread;
    pstd::optional<float> finish_time;

    std::function<void(std::string_view, int64_t, int64_t, float)> update_callback =
        [](std::string_view, int64_t, int64_t, float) {};

#ifdef SPECULA_BUILD_GPU_RENDERER
    std::vector<cudaEvent_t> gpu_events;
    std::atomic<size_t> gpu_events_launched_offset;
    int gpu_events_finished_offset;
#endif
  };
} // namespace specula

template <> struct fmt::formatter<specula::Timer> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Timer &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ Timer start={} ]", v.start);
  }
};

template <> struct fmt::formatter<specula::ProgressReporter> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::ProgressReporter &v, FormatContext &ctx) const {
    return format_to(
        ctx.out(), "[ ProgressReporter totalWork={} title={} timer={} workDone={} exitThread={} ]",
        v.total_work, v.title, v.timer, v.work_done, v.exit_thread);
  }
};

#endif // SPECULA_UTIL_PROGRESS_REPORTER_HPP
