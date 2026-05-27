#ifndef INCLUDE_PARALLEL_PARALLEL_HPP_
#define INCLUDE_PARALLEL_PARALLEL_HPP_

#include <cstdint>
#include <functional>

#include "util/vecmath/bounds2.hpp"
#include "util/vecmath/bounds2i_iterator.hpp"
#include "util/vecmath/tuple2.hpp"

namespace specula {
  void parallel_init(int num_threads = -1);
  void parallel_cleanup();

  int available_cores();
  int running_threads();

  void parallel_for(int64_t start, int64_t end, std::function<void(int64_t, int64_t)> func);

  inline void parallel_for(int64_t start, int64_t end, std::function<void(int64_t)> func) {
    parallel_for(start, end, [&func](int64_t start, int64_t end) {
      for (int64_t i = start; i < end; ++i) {
        func(i);
      }
    });
  }

  void parallel_for_2d(const Bounds2i &extent, std::function<void(Bounds2i)> func);

  inline void parallel_for_2d(const Bounds2i &extent, std::function<void(Point2i)> func) {
    parallel_for_2d(extent, [&func](Bounds2i bounds) {
      for (Point2i p : bounds) {
        func(p);
      }
    });
  }

  bool do_parallel_work();

  void for_each_thread(std::function<void(void)> func);
  void disable_thread_pool();
  void reenable_thread_pool();
} // namespace specula

#endif // INCLUDE_PARALLEL_PARALLEL_HPP_
