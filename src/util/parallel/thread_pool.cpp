#include "specula/util/parallel/thread_pool.hpp"

#include <mutex>

#include "specula/util/log.hpp"
#include "specula/util/parallel/atomic.hpp"
#include "specula/util/parallel/functions.hpp"
#include "specula/util/parallel/parallel_job.hpp"

specula::ThreadPool::ThreadPool(int n_threads) {
  for (int i = 0; i < n_threads - 1; ++i) {
    threads.emplace_back(&ThreadPool::worker, this);
  }
}

void specula::ThreadPool::worker() {
  LOG_DEBUG("Started execution in worker thread");

#ifdef SPECULA_BUILD_GPU_RENDERER
  gpu_thread_init();
#endif

  std::unique_lock<LockableBase(std::mutex)> lock(mutex);
  while (!shutdown_threads) {
    work_or_wait(&lock, false);
  }

  LOG_DEBUG("Exiting worker thread");
}

std::unique_lock<LockableBase(std::mutex)> specula::ThreadPool::add_to_job_list(ParallelJob *job) {
  std::unique_lock<LockableBase(std::mutex)> lock(mutex);
  if (job_list != nullptr) {
    job_list->prev = job;
  }
  job->next = job_list;
  job_list = job;

  job_list_condition.notify_all();
  return lock;
}

void specula::ThreadPool::work_or_wait(std::unique_lock<LockableBase(std::mutex)> *lock,
                                       bool is_enqueuing_thread) {
  DASSERT(lock->owns_lock());

  if (!is_enqueuing_thread && disabled) {
    job_list_condition.wait(*lock);
    return;
  }

  ParallelJob *job = job_list;
  while ((job != nullptr) && !job->have_work()) {
    job = job->next;
  }
  if (job != nullptr) {
    job->active_workders++;
    job->run_step(lock);
    DASSERT(!lock->owns_lock());
    lock->lock();
    job->active_workders--;
    if (job->finished()) {
      job_list_condition.notify_all();
    }
  } else {
    job_list_condition.wait(*lock);
  }
}

void specula::ThreadPool::remove_from_job_list(ParallelJob *job) {
  DASSERT(!job->removed);

  if (job->prev != nullptr) {
    job->prev->next = job->next;
  } else {
    DASSERT(job_list == job);
    job_list = job->next;
  }
  if (job->next != nullptr) {
    job->next->prev = job->prev;
  }
  job->removed = true;
}

bool specula::ThreadPool::work_or_return() {
  std::unique_lock<LockableBase(std::mutex)> lock(mutex);

  ParallelJob *job = job_list;
  while ((job != nullptr) && !job->have_work()) {
    job = job->next;
  }
  if (job == nullptr) {
    return false;
  }

  job->active_workders++;
  job->run_step(&lock);
  DASSERT(!lock.owns_lock());
  lock.lock();
  job->active_workders--;
  if (job->finished()) {
    job_list_condition.notify_all();
  }
  return true;
}

void specula::ThreadPool::for_each_thread(std::function<void(void)> func) {
  auto *barrier = new Barrier(threads.size() + 1);

  parallel_for(0, threads.size() + 1, [barrier, &func](int64_t) {
    func();
    if (barrier->block()) {
      delete barrier;
    }
  });
}

void specula::ThreadPool::disable() {
  ASSERT(!disabled);
  disabled = true;
  ASSERT(job_list == nullptr);
}

void specula::ThreadPool::reenable() {
  ASSERT(disabled);
  disabled = false;
}

specula::ThreadPool::~ThreadPool() {
  if (threads.empty()) {
    return;
  }

  {
    std::lock_guard<LockableBase(std::mutex)> lock(mutex);
    shutdown_threads = true;
    job_list_condition.notify_all();
  }

  for (std::thread &thread : threads) {
    thread.join();
  }
}
