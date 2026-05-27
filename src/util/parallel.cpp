#include <mutex>

#include <specula/util/parallel.hpp>

#include "util/parallel/atomic.hpp"
#include "util/parallel/thread_pool.hpp"

namespace specula {
  ThreadPool *ParallelJob::thread_pool = nullptr;
}

bool specula::Barrier::block() {
  std::unique_lock<std::mutex> lock(mutex_);
  --num_to_block_;
  ASSERT_GE(num_to_block_, 0);

  if (num_to_block_ > 0) {
    cv_.wait(lock, [this]() { return num_to_block_ == 0; });
  } else {
    cv_.notify_all();
  }

  return --num_to_exit_ == 0;
}

specula::ThreadPool::ThreadPool(size_t num_threads) {
  for (size_t i = 0; i < num_threads - 1; ++i) {
    threads_.push_back(std::thread(&ThreadPool::worker, this));
  }
}

void specula::ThreadPool::worker() {
  LOG_DEBUG("Started execution in worker thread");

#ifdef SPECULA_BUILD_GPU_RENDERER
  gpu_thread_init();
#endif

  std::unique_lock<std::mutex> lock(mutex_);
  while (!shutdown_threads_) {
    work_or_wait(&lock, false);
  }

  LOG_DEBUG("Exiting worker thread");
}
