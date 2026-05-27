#ifndef INCLUDE_PARALLEL_THREAD_LOCAL_HPP_
#define INCLUDE_PARALLEL_THREAD_LOCAL_HPP_

#include <shared_mutex>

#include <tracy/Tracy.hpp>

#include "util/parallel/parallel.hpp"
#include "util/pstd/optional.hpp"

namespace specula {
  template <typename T> class ThreadLocal {
  public:
    ThreadLocal() : hash_table_(4 * running_threads()), create_([]() { return T(); }) {}
    ThreadLocal(std::function<T(void)> &&c) : hash_table_(4 * running_threads()), create_(c) {}

    inline T &get() {
      std::thread::id tid = std::this_thread::get_id();
      uint32_t hash = std::hash<std::thread::id>()(tid);
      hash %= hash_table_.size();
      int step = 1;
      int tries = 0;

      mutex_.lock_shared();

      while (true) {
        ASSERT_LT(++tries, hash_table_.size());

        if (hash_table_[hash] && hash_table_[hash]->tid == tid) {
          T &val = hash_table_[hash]->value;
          mutex_.unlock_shared();
          return val;
        } else if (!hash_table_[hash]) {
          mutex_.unlock_shared();
          mutex_.lock();
          T new_val = create_();
          if (hash_table_[hash]) {
            while (true) {
              hash += step;
              ++step;
              if (hash >= hash_table_.size()) {
                hash %= hash_table_.size();
              }

              if (!hash_table_[hash]) {
                break;
              }
            }
          }

          hash_table_[hash] = Entry{tid, std::move(new_val)};
          T &val = hash_table_[hash]->value;
          mutex_.unlock();
          return val;
        }

        hash += step;
        ++step;
        if (hash >= hash_table_.size()) {
          hash %= hash_table_.size();
        }
      }
    }

    template <typename F> void for_all(F &&func) {
      mutex_.lock();

      for (auto &entry : hash_table_) {
        if (entry) {
          func(entry->value);
        }
      }

      mutex_.unlock();
    }

  private:
    struct Entry {
      std::thread::id tid;
      T value;
    };
    TracySharedLockable(std::shared_mutex, mutex_);
    std::vector<pstd::optional<Entry>> hash_table_;
    std::function<T(void)> create_;
  };
} // namespace specula

#endif // INCLUDE_PARALLEL_THREAD_LOCAL_HPP_
