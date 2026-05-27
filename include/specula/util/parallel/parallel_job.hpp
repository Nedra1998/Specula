#ifndef INCLUDE_PARALLEL_PARALLEL_JOB_HPP_
#define INCLUDE_PARALLEL_PARALLEL_JOB_HPP_

#include <condition_variable>
#include <mutex>

#include "parallel.hpp"
#include "thread_pool.hpp"
#include "util/check.hpp"
#include "util/pstd/optional.hpp"

namespace specula {

  class ParallelJob {
  public:
    virtual ~ParallelJob() { DASSERT(removed_); }

    virtual bool have_work() const = 0;
    virtual void run_step(std::unique_lock<std::mutex> *lock) = 0;

    bool finished() const { return !have_work() && active_workers_ == 0; }

    static ThreadPool *thread_pool;

  private:
    friend class ThreadPool;
    unsigned active_workers_ = 0;
    ParallelJob *prev_ = nullptr, *next_ = nullptr;
    bool removed_ = false;
  };

  template <typename T> class AsyncJob : public ParallelJob {
  public:
    AsyncJob(std::function<T(void)> func) : func_(std::move(func)) {}

    bool have_work() const { return !started_; }

    void run_step(std::unique_lock<std::mutex> *lock) {
      thread_pool->remove_from_job_list(this);
      started_ = true;
      lock->unlock();

      T result = func_();
      std::unique_lock<std::mutex> ul(mutex_);
      result_ = result;
      cv_.notify_all();
    }

    bool is_ready() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return result_.has_value();
    }

    void wait() {
      while (!is_ready() && do_parallel_work()) {
      }
      std::unique_lock<std::mutex> lock(mutex_);
      if (!result_.has_value()) {
        cv_.wait(lock, [this]() { return result_.has_value(); });
      }
    }

    T get_result() {
      wait();
      std::lock_guard<std::mutex> lock(mutex_);
      return *result_;
    }

    pstd::optional<T> try_get_result(std::mutex *mutex) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (result_) {
          return result_;
        }
      }

      mutex->unlock();
      do_parallel_work();
      mutex->lock();
      return {};
    }

    void do_work() {
      T r = func_();
      std::unique_lock<std::mutex> lock(mutex_);
      CHECK(!result_.has_value());
      result_ = r;
      cv_.notify_all();
    }

  private:
    std::function<T(void)> func_;
    bool started_ = false;
    pstd::optional<T> result_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
  };

  template <typename F, typename... Args> inline auto run_async(F func, Args &&...args) {
    auto fvoid = std::bind(func, std::forward<Args>(args)...);
    using R = typename std::invoke_result_t<F, Args...>;
    AsyncJob<R> *job = new AsyncJob<R>(std::move(fvoid));

    std::unique_lock<std::mutex> lock;
    if (running_threads() == 1) {
      job->do_work();
    } else {
      lock = ParallelJob::thread_pool->add_to_job_list(job);
    }

    return job;
  }

} // namespace specula

#endif // INCLUDE_PARALLEL_PARALLEL_JOB_HPP_
