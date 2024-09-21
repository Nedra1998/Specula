#include "util/progress_reporter.hpp"

#include <fmt/color.h>

#include "util/check.hpp"

specula::ProgressReporter::ProgressReporter(std::int64_t total_work, std::string title, bool quiet,
                                            bool gpu)
    : total_work(std::max<std::int64_t>(1, total_work)), title(title), quiet(quiet), work_done(0),
      exit_thread(false) {
#ifdef SPECULA_BUILD_GPU_RENDERER
  if (gpu) {
    gpu_events_launched_offset = 0;
    gpu_events_finished_offset = 0;
    gpu_events.resize(total_work);
    for (cudaEvent_t &event : gpu_events)
      CUDA_CHECK(cudaEventCreate(&event));
  }
#else
  ASSERT_EQ(gpu, false);
#endif

  if (!quiet) {
    update_thread = std::thread([this]() {
#ifdef SPECULA_BUILD_GPU_RENDERER
      GPURegisterThread("SPECULA_PRGRESS_BAR");
#endif

      run();
    });
  }
}

specula::ProgressReporter::~ProgressReporter() { done(); }

void specula::ProgressReporter::done() {
  if (!quiet) {
#ifdef SPECULA_BUILD_GPU_RENDERER
    if (gpu_events.size()) {
      while (gpu_events_finished_offset < gpu_events_launched_offset) {
        cudaError_t err = cudaEventSynchronize(gpu_events[gpu_events_finished_offset]);
        if (err != cudaSuccess)
          LOG_CRITICAL("CUDA error: {}", cudaGetErrorString(err));
      }
      work_done = gpu_events.size();
    }
#endif

    bool fa = false;
    finish_time = elapsed_seconds();
    if (exit_thread.compare_exchange_strong(fa, true)) {
      work_done = total_work;
      exit_thread = true;
      if (update_thread.joinable())
        update_thread.join();
    }
  }
}

void specula::ProgressReporter::run() {

#ifdef PBRT_BUILD_GPU_RENDERER
  std::chrono::milliseconds sleep_duration(gpu_events.size() ? 50 : 250);
#else
  std::chrono::milliseconds sleep_duration(250);
#endif

  int iter_count = 0;
  bool exit = false;

  while (!exit) {
    if (exit_thread)
      exit = true;
    else
      std::this_thread::sleep_for(sleep_duration);

    ++iter_count;

    // Periodically increase sleep duration to reduce overhead of updating the
    // progress bar.
    if (iter_count == 10)
      sleep_duration *= 2;
    else if (iter_count == 70)
      sleep_duration *= 2;
    else if (iter_count == 520)
      sleep_duration *= 5;

#ifdef SPECULA_BUILD_GPU_RENDERER
    if (gpu_events.size()) {
      while (gpu_events_finished_offset < gpu_events_launched_offset) {
        cudaError_t err = cudaEventQuery(gpu_events[gpu_events_finished_offset]);
        if (err == cudaSuccess)
          ++gpu_events_finished_offset;
        else if (err == cudaErrorNotReady)
          break;
        else
          LOG_CRITICAL("CUDA error: %s", cudaGetErrorString(err));
      }
      workDone = gpu_events_finished_offset;
    }
#endif

    update_callback(this->title, this->work_done, this->total_work, this->elapsed_seconds());
  }
}

std::string specula::ProgressReporter::print_bar(std::string_view title, std::int64_t work_done,
                                                 std::int64_t total_work, float elapsed,
                                                 bool color) {
  float percent_done = float(work_done) / float(total_work);
  float filled = percent_done * BAR_WIDTH;
  bool left_head = filled - std::floor(filled) < 0.5;

  float remaining = elapsed / percent_done - elapsed;

  std::uint8_t elapsed_hours = std::floor(elapsed / 3600);
  std::uint8_t elapsed_minutes = std::floor((elapsed - elapsed_hours * 3600) / 60);
  std::uint8_t elapsed_secs = elapsed - elapsed_hours * 3600 - elapsed_minutes * 60;

  std::uint8_t remaining_hours = std::floor(remaining / 3600);
  std::uint8_t remaining_minutes = std::floor((remaining - remaining_hours * 3600) / 60);
  std::uint8_t remaining_secs = remaining - remaining_hours * 3600 - remaining_minutes * 60;

  if (color) {
    if (percent_done == 1) {
      return fmt::format(
          "\r{} {} {} {}{}{}{}{}", title,
          fmt::format(fg(fmt::terminal_color::bright_green), "{:━<{}}", "", BAR_WIDTH),
          fmt::format(fg(fmt::terminal_color::magenta), "{:3.0f}%", 100.0f * percent_done),
          fmt::styled("[", fmt::emphasis::faint | fmt::fg(fmt::terminal_color::blue)),
          fmt::format(fg(fmt::terminal_color::blue), "{:02d}:{:02d}:{:02d}", elapsed_hours,
                      elapsed_minutes, elapsed_secs),
          fmt::styled("<", fmt::emphasis::faint | fmt::fg(fmt::terminal_color::blue)),
          fmt::format(fg(fmt::terminal_color::blue), "{:02d}:{:02d}:{:02d}", remaining_hours,
                      remaining_minutes, remaining_secs),
          fmt::styled("]", fmt::emphasis::faint | fmt::fg(fmt::terminal_color::blue)));
    } else {
      return fmt::format(
          "\r{} {}{}{} {} {}{}{}{}{}", title,
          fmt::format(fg(fmt::terminal_color::bright_red), "{:━<{}}", "",
                      static_cast<std::uint8_t>(filled)),
          fmt::styled(left_head ? "╺" : "╸", left_head ? fmt::fg(fmt::terminal_color::bright_black)
                                                       : fmt::fg(fmt::terminal_color::bright_red)),
          fmt::format(fg(fmt::terminal_color::bright_black), "{:━<{}}", "",
                      BAR_WIDTH - static_cast<std::uint8_t>(filled) - 1),
          fmt::format(fg(fmt::terminal_color::magenta), "{:3.0f}%", 100.0f * percent_done),
          fmt::styled("[", fmt::emphasis::faint | fmt::fg(fmt::terminal_color::blue)),
          fmt::format(fg(fmt::terminal_color::blue), "{:02d}:{:02d}:{:02d}", elapsed_hours,
                      elapsed_minutes, elapsed_secs),
          fmt::styled("<", fmt::emphasis::faint | fmt::fg(fmt::terminal_color::blue)),
          fmt::format(fg(fmt::terminal_color::blue), "{:02d}:{:02d}:{:02d}", remaining_hours,
                      remaining_minutes, remaining_secs),
          fmt::styled("]", fmt::emphasis::faint | fmt::fg(fmt::terminal_color::blue)));
    }
  } else {
    if (percent_done == 1) {
      return fmt::format(
          "\r{} {} {} {}{}{}{}{}", title, fmt::format("{:━<{}}", "", BAR_WIDTH),
          fmt::format("{:3.0f}%", 100.0f * percent_done), "[",
          fmt::format("{:02d}:{:02d}:{:02d}", elapsed_hours, elapsed_minutes, elapsed_secs), "<",
          fmt::format("{:02d}:{:02d}:{:02d}", remaining_hours, remaining_minutes, remaining_secs),
          "]");
    } else {
      return fmt::format(
          "\r{} {}{}{} {} {}{}{}{}{}", title,
          fmt::format("{:━<{}}", "", static_cast<std::uint8_t>(filled)), left_head ? "╺" : "╸",
          fmt::format("{:━<{}}", "", BAR_WIDTH - static_cast<std::uint8_t>(filled) - 1),
          fmt::format("{:3.0f}%", 100.0f * percent_done), "[",
          fmt::format("{:02d}:{:02d}:{:02d}", elapsed_hours, elapsed_minutes, elapsed_secs), "<",
          fmt::format("{:02d}:{:02d}:{:02d}", remaining_hours, remaining_minutes, remaining_secs),
          "]");
    }
  }
}
