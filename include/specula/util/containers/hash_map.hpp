#ifndef SPECULA_UTIL_CONTAINERS_HASH_MAP_HPP
#define SPECULA_UTIL_CONTAINERS_HASH_MAP_HPP

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/pstd.hpp"

namespace specula {
  template <typename Key, typename Value, typename Hash = std::hash<Key>,
            typename Allocator =
                pstd::pmr::polymorphic_allocator<pstd::optional<std::pair<Key, Value>>>>
  class HashMap {
  public:
    using TableEntry = pstd::optional<std::pair<Key, Value>>;

    class Iterator {
    public:
      SPECULA_CPU_GPU Iterator &operator++() {
        while (++ptr < end && !ptr->has_value()) {
        }
        return *this;
      }
      SPECULA_CPU_GPU Iterator operator++(int) {
        Iterator old = *this;
        operator++();
        return old;
      }

      SPECULA_CPU_GPU bool operator==(const Iterator &iter) const { return ptr == iter.ptr; }
      SPECULA_CPU_GPU bool operator!=(const Iterator &iter) const { return ptr != iter.ptr; }

      SPECULA_CPU_GPU std::pair<Key, Value> &operator*() { return ptr->value(); }
      SPECULA_CPU_GPU const std::pair<Key, Value> &operator*() const { return ptr->value(); }

      SPECULA_CPU_GPU std::pair<Key, Value> *operator->() { return &ptr->value(); }
      SPECULA_CPU_GPU const std::pair<Key, Value> *operator->() const { return ptr->value(); }

    private:
      friend class HashMap;
      Iterator(TableEntry *ptr, TableEntry *end) : ptr(ptr), end(end) {}
      TableEntry *ptr, *end;
    };

    using iterator = Iterator;
    using const_iterator = const iterator;

    HashMap(Allocator alloc) : table(8, alloc) {}
    HashMap(const HashMap &) = delete;
    HashMap(HashMap &&) = default;

    ~HashMap() = default;

    HashMap &operator=(const HashMap &) = delete;
    HashMap &operator=(HashMap &&) = default;

    SPECULA_CPU_GPU [[nodiscard]] size_t size() const { return n_stored; }
    SPECULA_CPU_GPU [[nodiscard]] size_t capacity() const { return table.size(); }

    void clear() {
      table.clear();
      n_stored = 0;
    }

    void insert(const Key &key, const Value &value) {
      size_t offset = find_offset(key);
      if (table[offset].has_value() == false) {
        if (3 * ++n_stored > capacity()) {
          grow();
          offset = find_offset(key);
        }
      }
      table[offset] = std::make_pair(key, value);
    }

    SPECULA_CPU_GPU bool has_key(const Key &key) const {
      return table[find_offset(key)].has_value();
    }

    SPECULA_CPU_GPU const Value &operator[](const Key &key) const {
      size_t offset = find_offset(key);
      ASSERT(table[offset].has_value());
      return table[offset]->second;
    }

    SPECULA_CPU_GPU iterator begin() {
      Iterator iter(table.data(), table.data() + capacity());
      while (iter.ptr < iter.end && !iter.ptr->has_value()) {
        ++iter.ptr;
      }
      return iter;
    }

    SPECULA_CPU_GPU iterator end() {
      return Iterator(table.data() + capacity(), table.data() + capacity());
    }

  private:
    SPECULA_CPU_GPU size_t find_offset(const Key &key) const {
      size_t base_offset = Hash()(key) & (capacity() - 1);
      for (int n_probes = 0;; ++n_probes) {
        size_t offset = (base_offset + n_probes / 2 + n_probes * n_probes / 2) & (capacity() - 1);
        if (table[offset].has_value() == false || key == table[offset]->first) {
          return offset;
        }
      }
    }

    void grow() {
      size_t current_capacity = capacity();
      pstd::vector<TableEntry> new_table(std::max<size_t>(64, 2 * current_capacity),
                                         table.get_allocator());
      size_t new_capacity = new_table.size();

      for (size_t i = 0; i < current_capacity; ++i) {
        if (!table[i].has_value()) {
          continue;
        }
        size_t base_offset = Hash()(table[i]->first) & (new_capacity - 1);
        for (int n_probes = 0;; ++n_probes) {
          size_t offset =
              (base_offset + n_probes / 2 + n_probes * n_probes / 2) & (new_capacity - 1);
          if (!new_table[offset]) {
            new_table[offset] = std::move(*table[i]);
            break;
          }
        }
      }
      table = std::move(new_table);
    }

    pstd::vector<TableEntry> table;
    size_t n_stored = 0;
  };

} // namespace specula

#endif // SPECULA_UTIL_CONTAINERS_HASH_MAP_HPP
