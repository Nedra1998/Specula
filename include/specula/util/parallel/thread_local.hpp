#ifndef SPECULA_UTIL_PARALLEL_THREAD_LOCAL_HPP
#define SPECULA_UTIL_PARALLEL_THREAD_LOCAL_HPP

#include <shared_mutex>

#include <tracy/Tracy.hpp>

#include "specula/util/parallel/functions.hpp"
#include "specula/util/pstd/optional.hpp"

namespace specula {
  template <typename T> class ThreadLocal {
  public:
    ThreadLocal() : hash_table(4 * running_threads()), create([]() { return T(); }) {}
    ThreadLocal(std::function<T(void)> &&c) : hash_table(4 * running_threads()), create(c) {}

    T &get() {
      std::thread::id tid = std::this_thread::get_id();
      uint32_t hash = std::hash<std::thread::id>()(tid);
      hash %= hash_table.size();
      int step = 1;
      int tries = 0;

      mutex.lock_shared();
      while (true) {
        ASSERT_LT(++tries, hash_table.size());
        if (hash_table[hash] && hash_table[hash]->tid == tid) {
          T &thread_local_value = hash_table[hash]->value;
          mutex.unlock_shared();
          return thread_local_value;
        } else if (!hash_table[hash]) {
          mutex.unlock_shared();

          mutex.lock();
          T new_item = create();
          if (hash_table[hash]) {
            while (true) {
              hash += step;
              ++step;
              if (hash >= hash_table.size()) {
                hash %= hash_table.size();
              }
              if (!hash_table[hash]) {
                break;
              }
            }
          }

          hash_table[hash] = Entry{tid, std::move(new_item)};
          T &thread_local_value = hash_table[hash]->value;
          mutex.unlock();
          return thread_local_value;
        }

        hash += step;
        ++step;
        if (hash >= hash_table.size()) {
          hash %= hash_table.size();
        }
      }
    }

    template <typename F> void for_all(F &&func) {
      mutex.lock();
      for (auto &entry : hash_table) {
        if (entry) {
          func(entry->value);
        }
      }
      mutex.unlock();
    }

  private:
    struct Entry {
      std::thread::id tid;
      T value;
    };

    TracySharedLockable(std::shared_mutex, mutex);
    std::vector<pstd::optional<Entry>> hash_table;
    std::function<T(void)> create;
  };
} // namespace specula

#endif // SPECULA_UTIL_PARALLEL_THREAD_LOCAL_HPP
