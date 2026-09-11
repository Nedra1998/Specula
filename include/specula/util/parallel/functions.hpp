#ifndef SPECULA_UTIL_PARALLEL_FUNCTIONS_HPP
#define SPECULA_UTIL_PARALLEL_FUNCTIONS_HPP

#include <cstdint>
#include <functional>

#include "specula/util/vecmath/bounds2.hpp"
#include "specula/util/vecmath/tuple2.hpp"

namespace specula {
  int available_cores();
  int running_threads();

  void parallel_init(int nThreads = -1);
  void parallel_cleanup();

  void parallel_for(int64_t start, int64_t end, std::function<void(int64_t, int64_t)> func);
  void parallel_for_2d(const Bounds2i &extent, std::function<void(Bounds2i)> func);

  inline void parallel_for(int64_t start, int64_t end, std::function<void(int64_t)> func) {
    parallel_for(start, end, [&func](int64_t start, int64_t end) {
      for (int64_t i = start; i < end; ++i) {
        func(i);
      }
    });
  }

  inline void parallel_for_2d(const Bounds2i &extent, std::function<void(Point2i)> func) {
    parallel_for_2d(extent, [&func](Bounds2i b) {
      for (Point2i p : b) {
        func(p);
      }
    });
  }

  bool do_parallel_work();

  void for_each_thread(std::function<void(void)> func);
  void disable_thread_pool();
  void reenable_thread_pool();
} // namespace specula

#endif // SPECULA_UTIL_PARALLEL_FUNCTIONS_HPP
