#ifndef SPECULA_UTIL_PARALLEL_PARALLEL_JOB_HPP
#define SPECULA_UTIL_PARALLEL_PARALLEL_JOB_HPP

#include <condition_variable>
#include <mutex>
#include <type_traits>

#include <fmt/format.h>

#include "specula/util/check.hpp"
#include "specula/util/parallel/functions.hpp"
#include "specula/util/parallel/thread_pool.hpp"
#include "specula/util/pstd/optional.hpp"

namespace specula {
  class ParallelJob {
  public:
    virtual ~ParallelJob() { DASSERT(removed); }

    [[nodiscard]] virtual bool have_work() const = 0;
    virtual void run_step(std::unique_lock<LockableBase(std::mutex)> *lock) = 0;

    [[nodiscard]] bool finished() const { return !have_work() && active_workders == 0; }

    static ThreadPool *thread_pool;

  private:
    [[nodiscard]] inline virtual std::string format() const {
      return fmt::format("[ ParallelJob activeWorkers={} removed={} ]", active_workders, removed);
    }

    friend class ThreadPool;
    int active_workders = 0;
    ParallelJob *prev = nullptr, *next = nullptr;
    bool removed = false;

    friend struct fmt::formatter<ParallelJob>;
  };

  template <typename T> class AsyncJob : public ParallelJob {
  public:
    AsyncJob(std::function<T(void)> w) : func(std::move(w)) {}

    bool have_work() const override { return !started; }
    void run_step(std::unique_lock<LockableBase(std::mutex)> *lock) override {
      thread_pool->remove_from_job_list(this);
      started = true;
      lock->unlock();
      T r = func();
      std::unique_lock<LockableBase(std::mutex)> ul(mutex);
      result = r;
      cv.notify_all();
    }

    bool is_ready() const {
      std::lock_guard<LockableBase(std::mutex)> lock(mutex);
      return result.has_value();
    }

    T get_result() {
      wait();
      std::lock_guard<LockableBase(std::mutex)> lock(mutex);
      return *result;
    }

    pstd::optional<T> try_get_result(LockableBase(std::mutex) * ext_mutex) {
      {
        std::lock_guard<LockableBase(std::mutex)> lock(mutex);
        if (result) {
          return result;
        }
      }
      ext_mutex->unlock();
      do_parallel_work();
      ext_mutex->lock();
      return {};
    }

    void wait() {
      while (!is_ready() && do_parallel_work()) {
      }

      std::unique_lock<LockableBase(std::mutex)> lock(mutex);
      if (!result.has_value()) {
        cv.wait(lock, [this]() { return result.has_value(); });
      }
    }

    void do_work() {
      T r = func();
      std::unique_lock<LockableBase(std::mutex)> lock(mutex);
      ASSERT(!result.has_value());
      result = r;
      cv.notify_all();
    }

  private:
    inline std::string format() const override {
      return fmt::format("[ AsyncJob started={} ]", started);
    }

    std::function<T(void)> func;
    bool started = false;
    pstd::optional<T> result;
    mutable TracyLockable(std::mutex, mutex);
    std::condition_variable cv;

    friend struct fmt::formatter<AsyncJob<T>>;
  };

  template <typename F, typename... Args> inline auto run_async(F func, Args &&...args) {
    auto fvoid = std::bind(func, std::forward<Args>(args)...);
    using R = typename std::invoke_result_t<F, Args...>;
    auto *job = new AsyncJob<R>(std::move(fvoid));

    std::unique_lock<LockableBase(std::mutex)> lock;
    if (running_threads() == 1) {
      job->do_work();
    } else {
      lock = ParallelJob::thread_pool->add_to_job_list(job);
    }
    return job;
  }
} // namespace specula

template <> struct fmt::formatter<specula::ParallelJob> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::ParallelJob &v, FormatContext &ctx) const {
    return format_to(ctx.out(), v.format());
  }
};

template <typename T> struct fmt::formatter<specula::AsyncJob<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::AsyncJob<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), v.format());
  }
};

template <> struct fmt::formatter<specula::ThreadPool> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::ThreadPool &v, FormatContext &ctx) const {
    format_to(ctx.out(),
              "[ ThreadPool threads.size()={} shutdownThreads={} jobList=", v.threads.size(),
              v.shutdown_threads);
    if (v.mutex.try_lock()) {
      format_to(ctx.out(), "[ ");
      specula::ParallelJob *job = v.job_list;
      while (job != nullptr) {
        format_to(ctx.out(), job->format());
        job = job->next;
        if (job != nullptr) {
          format_to(ctx.out(), ", ");
        }
      }
      format_to(ctx.out(), " ]");
    } else {
      format_to(ctx.out(), "<job list mutex locked>");
    }

    return format_to(ctx.out(), " ]");
  }
};

#endif // SPECULA_UTIL_PARALLEL_PARALLEL_JOB_HPP
