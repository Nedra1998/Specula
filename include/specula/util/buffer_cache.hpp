#ifndef SPECULA_UTIL_BUFFER_CACHE_HPP
#define SPECULA_UTIL_BUFFER_CACHE_HPP

#include <atomic>
#include <shared_mutex>
#include <unordered_set>

#include "specula/util/check.hpp"
#include "specula/util/pstd.hpp"
#include "specula/util/stats.hpp"
#include "specula/util/vecmath.hpp"

namespace specula {
  STAT_MEMORY_COUNTER("Memory/Redundant vertex and index buffers", REDUNDANT_BUFFER_BYTES);
  STAT_PERCENT("Geometry/Buffer cache hits", N_BUFFER_CACHE_HITS, N_BUFFER_CACHE_LOOKUPS);

  template <typename T> class BufferCache {
  public:
    const T *lookup_or_add(pstd::span<const T> buf, Allocator alloc) {
      ++N_BUFFER_CACHE_LOOKUPS;
      Buffer lookup_buffer(buf.data(), buf.size());
      int shard_index = uint32_t(lookup_buffer.hash) >> (32 - LOG_SHARDS);
      DASSERT(shard_index >= 0 && shard_index < N_SHARDS);
      mutex[shard_index].lock_shared();
      if (auto iter = cache[shard_index].find(lookup_buffer); iter != cache[shard_index].end()) {
        const T *ptr = iter->ptr;
        mutex[shard_index].unlock_shared();
        DASSERT(std::memcmp(buf.data(), iter->ptr, buf.size() * sizeof(T)) == 0);
        ++N_BUFFER_CACHE_HITS;
        REDUNDANT_BUFFER_BYTES += buf.size() * sizeof(T);
        return ptr;
      }

      mutex[shard_index].unlock_shared();
      T *ptr = alloc.allocate_object<T>(buf.size());
      std::copy(buf.begin(), buf.end(), ptr);
      bytes_used_ += buf.size() * sizeof(T);
      mutex[shard_index].lock();
      if (auto iter = cache[shard_index].find(lookup_buffer); iter != cache[shard_index].end()) {
        const T *cache_ptr = iter->ptr;
        mutex[shard_index].unlock();
        alloc.deallocate_object(ptr, buf.size());
        ++N_BUFFER_CACHE_HITS;
        REDUNDANT_BUFFER_BYTES += buf.size() * sizeof(T);
        return cache_ptr;
      }

      cache[shard_index].insert(Buffer(ptr, buf.size()));
      mutex[shard_index].unlock();
      return ptr;
    }

    [[nodiscard]] size_t bytes_used() const { return bytes_used_; }

  private:
    struct Buffer {
      Buffer() = default;
      Buffer(const T *ptr, size_t size) : ptr(ptr), size(size), hash(HashBuffer(ptr, size)) {}

      bool operator==(const Buffer &b) const {
        return size == b.size && hash == b.hash && std::memcmp(ptr, b.ptr, size * sizeof(T)) == 0;
      }

      const T *ptr = nullptr;
      size_t size = 0, hash{};
    };

    struct BufferHasher {
      size_t operator()(const Buffer &b) const { return b.hash; }
    };

    static constexpr int LOG_SHARDS = 6;
    static constexpr int N_SHARDS = 1 << LOG_SHARDS;

    // TODO: Figure out how to wrap this in a TracySharedLockable
    std::shared_mutex mutex[N_SHARDS];
    std::unordered_set<Buffer, BufferHasher> cache[N_SHARDS];
    std::atomic<size_t> bytes_used_;
  };

  extern BufferCache<int> *INT_BUFFER_CACHE;
  extern BufferCache<Point2f> *POINT2_BUFFER_CACHE;
  extern BufferCache<Point3f> *POINT3_BUFFER_CACHE;
  extern BufferCache<Vector3f> *VECTOR3_BUFFER_CACHE;
  extern BufferCache<Normal3f> *NORMAL3_BUFFER_CACHE;

  void init_buffer_caches();
} // namespace specula

#endif // SPECULA_UTIL_BUFFER_CACHE_HPP
