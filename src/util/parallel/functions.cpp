#include "specula/util/parallel/functions.hpp"

#include <cstdint>
#include <mutex>
#include <thread>

#include "specula/util/check.hpp"
#include "specula/util/parallel/parallel_job.hpp"
#include "specula/util/parallel/thread_pool.hpp"
#include "specula/util/vecmath/bounds2.hpp"
#include "specula/util/vecmath/tuple2.hpp"

class ParallelForLoop1D : public specula::ParallelJob {
public:
  ParallelForLoop1D(int64_t start, int64_t end, int chunk_size,
                    std::function<void(int64_t, int64_t)> func)
      : func(std::move(func)), next_index(start), end_index(end), chunk_size(chunk_size) {}

  [[nodiscard]] bool have_work() const override { return next_index < end_index; }
  void run_step(std::unique_lock<LockableBase(std::mutex)> *lock) override {
    int64_t index_start = next_index;
    int64_t index_end = std::min(index_start + chunk_size, end_index);
    next_index = index_end;

    if (!have_work()) {
      thread_pool->remove_from_job_list(this);
    }
    lock->unlock();
    func(index_start, index_end);
  }

  [[nodiscard]] inline std::string format() const override {
    return fmt::format("[ ParallelForLoop1D nextIndex={} endIndex={} chunkSize={} ]", next_index,
                       end_index, chunk_size);
  }

private:
  std::function<void(int64_t, int64_t)> func;
  int64_t next_index, end_index;
  int chunk_size;
};

class ParallelForLoop2D : public specula::ParallelJob {
public:
  ParallelForLoop2D(const specula::Bounds2i &extent, int chunk_size,
                    std::function<void(specula::Bounds2i)> func)
      : func(std::move(func)), extent(extent), next_start(extent.p_min), chunk_size(chunk_size) {}

  [[nodiscard]] bool have_work() const override { return next_start.y < extent.p_max.y; }
  void run_step(std::unique_lock<LockableBase(std::mutex)> *lock) override {
    specula::Point2i end = next_start + specula::Vector2i(chunk_size, chunk_size);
    specula::Bounds2i b = specula::intersect(specula::Bounds2i(next_start, end), extent);
    ASSERT(!b.is_empty());

    next_start.x += chunk_size;
    if (next_start.x >= extent.p_max.x) {
      next_start.x = extent.p_min.x;
      next_start.y += chunk_size;
    }

    if (!have_work()) {
      thread_pool->remove_from_job_list(this);
    }

    lock->unlock();
    func(b);
  }

  [[nodiscard]] inline std::string format() const override {
    return fmt::format("[ ParallelForLoop2D extent={} nextStart={} chunkSize={} ]", extent,
                       next_start, chunk_size);
  }

private:
  std::function<void(specula::Bounds2i)> func;
  const specula::Bounds2i extent;
  specula::Point2i next_start;
  int chunk_size;
};

bool specula::do_parallel_work() {
  ASSERT(ParallelJob::thread_pool);
  return ParallelJob::thread_pool->work_or_return();
}

void specula::parallel_for(int64_t start, int64_t end, std::function<void(int64_t, int64_t)> func) {
  ASSERT(ParallelJob::thread_pool);
  if (start == end) {
    return;
  }

  int64_t chunk_size = std::max<int64_t>(1, (end - start) / (8 * running_threads()));

  ParallelForLoop1D loop(start, end, chunk_size, std::move(func));
  std::unique_lock<LockableBase(std::mutex)> lock =
      ParallelJob::thread_pool->add_to_job_list(&loop);

  while (!loop.finished()) {
    ParallelJob::thread_pool->work_or_wait(&lock, true);
  }
}

void specula::parallel_for_2d(const Bounds2i &extent, std::function<void(Bounds2i)> func) {
  ASSERT(ParallelJob::thread_pool);
  if (extent.is_empty()) {
    return;
  }
  if (extent.area() == 1) {
    func(extent);
    return;
  }

  int tile_size = clamp(
      int(std::sqrt(extent.diagonal().x * extent.diagonal().y / (8 * running_threads()))), 1, 32);

  ParallelForLoop2D loop(extent, tile_size, std::move(func));
  std::unique_lock<LockableBase(std::mutex)> lock =
      ParallelJob::thread_pool->add_to_job_list(&loop);

  while (!loop.finished()) {
    ParallelJob::thread_pool->work_or_wait(&lock, true);
  }
}

int specula::available_cores() { return std::max<int>(1, std::thread::hardware_concurrency()); }

int specula::running_threads() {
  return (ParallelJob::thread_pool != nullptr) ? (1 + ParallelJob::thread_pool->size()) : 1;
}

void specula::parallel_init(int n_threads) {
  ASSERT(!ParallelJob::thread_pool);
  if (n_threads <= 0) {
    n_threads = available_cores();
  }
  ParallelJob::thread_pool = new ThreadPool(n_threads);
}

void specula::parallel_cleanup() {
  delete ParallelJob::thread_pool;
  ParallelJob::thread_pool = nullptr;
}

void specula::for_each_thread(std::function<void(void)> func) {
  if (ParallelJob::thread_pool != nullptr) {
    ParallelJob::thread_pool->for_each_thread(std::move(func));
  }
}

void specula::disable_thread_pool() {
  ASSERT(ParallelJob::thread_pool);
  ParallelJob::thread_pool->disable();
}
void specula::reenable_thread_pool() {
  ASSERT(ParallelJob::thread_pool);
  ParallelJob::thread_pool->reenable();
}
