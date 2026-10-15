#include "specula/util/progress_reporter.hpp"

specula::ProgressReporter::ProgressReporter(int64_t total_work, std::string title, bool quiet,
                                            bool gpu)
    : total_work(std::max<int64_t>(1, total_work)), title(std::move(title)), quiet(quiet),
      work_done(0), exit_thread(false) {
#ifdef SPECULA_BUILD_GPU_RENDERER
  if (gpu) {
    gpu_events_launched_offset = 0;
    gpu_events_finished_offset = 0;
    gpu_events.resize(total_work);
    for (cudeEvent_t &event : gpu_events) {
      CUDA_CHECK(cudeEventCreate(&event));
    }
  }
#else
  ASSERT(gpu == false);
#endif

  if (!quiet) {
    update_thread = std::thread([this]() {
#ifdef SPECULA_BUILD_GPU_RENDERER
      GpuRegisterThread("SPECULA_PROGRESS_BAR");
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
      if (update_thread.joinable()) {
        update_thread.join();
      }
    }
  }
}

void specula::ProgressReporter::run() {
#ifdef SPECULA_BUILD_GPU_RENDERER
  std::chrono::milliseconds sleep_duration(gpu_events.size() ? 50 : 250);
#else
  std::chrono::milliseconds sleep_duration(250);
#endif

  int iter_count = 0;
  bool really_exit = false;
  while (!really_exit) {
    if (exit_thread) {
      really_exit = true;
    } else {
      std::this_thread::sleep_for(sleep_duration);
    }

    ++iter_count;

    if (iter_count == 10) {
      sleep_duration *= 2;
    } else if (iter_count == 70) {
      sleep_duration *= 2;
    } else if (iter_count == 520) {
      sleep_duration *= 5;
    }

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

    update_callback(title, work_done, total_work, elapsed_seconds());
  }
}
