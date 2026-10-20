#include "specula/util/parallel/thread_local.hpp"

#include <atomic>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "specula/util/parallel.hpp"

using namespace specula;

TEST_CASE("ThreadLocal", "[util][parallel]") {
  SECTION("Consistency") {
    ThreadLocal<std::thread::id> tids([]() { return std::this_thread::get_id(); });

    std::atomic<int64_t> dummy{0};
    auto busywork = [&dummy](int64_t index) {
      int64_t result = 0;
      for (int64_t i = 0; i <= index; ++i) {
        for (int64_t j = 0; j <= index; ++j) {
          result += i * j;
        }
      }
      dummy.fetch_add(result, std::memory_order_relaxed);
    };

    parallel_for(0, 1000, [&](int64_t index) {
      CHECK(tids.get() == std::this_thread::get_id());
      busywork(index);
    });

    std::vector<AsyncJob<int> *> jobs;
    jobs.reserve(100);
    for (int i = 0; i < 100; ++i) {
      jobs.push_back(run_async([&]() {
        CHECK(tids.get() == std::this_thread::get_id());
        busywork(i);
        return i;
      }));
    }
    for (int i = 0; i < 100; ++i) {
      (void)jobs[i]->get_result();
    }

    parallel_for(0, 1000, [&](int64_t index) {
      CHECK(tids.get() == std::this_thread::get_id());
      busywork(index);
    });
  }
}
