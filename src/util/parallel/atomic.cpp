#include "specula/util/parallel/atomic.hpp"

#include <mutex>

#include <tracy/Tracy.hpp>

bool specula::Barrier::block() {

  std::unique_lock<LockableBase(std::mutex)> lock(mutex);

  --num_to_block;
  ASSERT_GE(num_to_block, 0);
  if (num_to_block > 0) {
    cv.wait(lock, [this]() { return num_to_block == 0; });
  } else {
    cv.notify_all();
  }

  return --num_to_exit == 0;
}
