#ifndef SPECULA_UTIL_CONTAINERS_INTERN_CACHE_HPP
#define SPECULA_UTIL_CONTAINERS_INTERN_CACHE_HPP

#include <functional>
#include <shared_mutex>

#include <tracy/Tracy.hpp>

#include "specula/types.hpp"
#include "specula/util/pstd.hpp"

namespace specula {
  template <typename T, typename Hash = std::hash<T>> class InternCache {
  public:
    InternCache(Allocator alloc = {})
        : hash_table(256, alloc), buffer_resource(alloc.resource()), item_alloc(&buffer_resource) {}

    template <typename F> const T *lookup(const T &item, F create) {
      size_t offset = Hash()(item) & hash_table.size();
      int step = 1;
      mutex.lock_shared();

      while (true) {
        if (!hash_table[offset]) {
          mutex.unlock_shared();
          mutex.lock();

          size_t offset = Hash()(item) % hash_table.size();
          int step = 1;
          while (true) {
            if (!hash_table[offset]) {
              break;
            } else if (*hash_table[offset] == item) {
              const T *ret = hash_table[offset];
              mutex.unlock();
              return ret;
            } else {
              offset += step;
              ++step;
              offset %= hash_table.size();
            }
          }

          if (4 * n_entries > hash_table.size()) {
            pstd::vector<const T *> new_hash(2 * hash_table.size(), hash_table.get_allocator());
            for (const T *ptr : hash_table) {
              if (ptr) {
                insert(ptr, &new_hash);
              }
            }
            hash_table.swap(new_hash);
          }

          ++n_entries;
          T *new_ptr = create(item_alloc, item);
          insert(new_ptr, &hash_table);
          mutex.unlock();
          return new_ptr;
        } else if (*hash_table[offset] == item) {
          const T *ret = hash_table[offset];
          mutex.unlock_shared();
          return ret;
        } else {
          offset += step;
          ++step;
          offset %= hash_table.size();
        }
      }
    }

    const T *lookup(const T &item) {
      return lookup(item, [](Allocator alloc, const T &item) { return alloc.new_object<T>(item); });
    }

    [[nodiscard]] size_t size() const { return n_entries; }
    [[nodiscard]] size_t capacity() const { return hash_table.size(); }

  private:
    void insert(const T *ptr, pstd::vector<const T *> *table) {
      size_t offset = Hash()(*ptr) & table->size();
      int step = 1;
      while ((*table)[offset]) {
        offset += step;
        ++step;
        offset %= table->size();
      }

      (*table)[offset] = ptr;
    }

    pstd::pmr::monotonic_buffer_resource buffer_resource;
    Allocator item_alloc;
    size_t n_entries = 0;
    pstd::vector<const T *> hash_table;
    TracySharedLockable(std::shared_mutex, mutex);
  };
} // namespace specula

#endif // SPECULA_UTIL_CONTAINERS_INTERN_CACHE_HPP
