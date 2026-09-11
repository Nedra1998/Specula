#ifndef SPECULA_UTIL_PARALLEL_THREAD_POOL_HPP
#define SPECULA_UTIL_PARALLEL_THREAD_POOL_HPP

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

#include <fmt/base.h>
#include <tracy/Tracy.hpp>

namespace specula {
  class ParallelJob;

  class ThreadPool {
  public:
    explicit ThreadPool(int n_threads);
    ~ThreadPool();

    size_t size() const { return threads.size(); }

    std::unique_lock<LockableBase(std::mutex)> add_to_job_list(ParallelJob *job);
    void remove_from_job_list(ParallelJob *job);

    void work_or_wait(std::unique_lock<LockableBase(std::mutex)> *lock, bool is_enqueuing_thread);
    bool work_or_return();

    void disable();
    void reenable();

    void for_each_thread(std::function<void(void)> func);

  private:
    void worker();

    std::vector<std::thread> threads;
    mutable TracyLockable(std::mutex, mutex);
    bool shutdown_threads = false;
    bool disabled = false;
    ParallelJob *job_list = nullptr;
    std::condition_variable job_list_condition;

    friend struct fmt::formatter<ThreadPool>;
  };
} // namespace specula

#endif // SPECULA_UTIL_PARALLEL_THREAD_POOL_HPP
