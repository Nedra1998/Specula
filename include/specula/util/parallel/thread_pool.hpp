#ifndef INCLUDE_PARALLEL_THREAD_POOL_HPP_
#define INCLUDE_PARALLEL_THREAD_POOL_HPP_

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace specula {
  class ParallelJob;

  class ThreadPool {
  public:
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    size_t size() const { return threads_.size(); }
    std::unique_lock<std::mutex> add_to_job_list(ParallelJob *job);
    void remove_from_job_list(ParallelJob *job);

    void work_or_wait(std::unique_lock<std::mutex> *lock, bool is_enqueuing_thread);
    bool work_or_return();

    void disable();
    void reenable();

    void for_each_thread(std::function<void(void)> func);

  private:
    void worker();

    std::vector<std::thread> threads_;
    mutable std::mutex mutex_;
    bool shutdown_threads_ = false;
    bool disabled_ = false;
    ParallelJob *job_list = nullptr;
    std::condition_variable job_list_condition_;
  };

} // namespace specula

#endif // INCLUDE_PARALLEL_THREAD_POOL_HPP_
