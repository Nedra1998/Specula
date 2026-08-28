#ifndef SPECULA_UTIL_MEMORY_HPP
#define SPECULA_UTIL_MEMORY_HPP

#include <atomic>
#include <list>

#include <spdlog/common.h>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/pstd/pmr.hpp"

namespace specula {
  template <typename T> struct AllocationTraits {
    using SingleObject = T *;
  };
  template <typename T> struct AllocationTraits<T[]> {
    using Array = T *;
  };
  template <typename T, size_t N> struct AllocationTraits<T[N]> {
    struct Invalid {};
  };

  class TrackedMemoryResource : public pstd::pmr::memory_resource {
  public:
    TrackedMemoryResource(pstd::pmr::memory_resource *source = pstd::pmr::get_default_resource())
        : source(source) {}

    void *do_allocate(size_t bytes, size_t alignment) override {
      void *ptr = source->allocate(bytes, alignment);
      uint64_t current_bytes = allocated_bytes.fetch_add(bytes) + bytes;
      uint64_t prev_max = max_allocated.load(std::memory_order_relaxed);
      while (prev_max < current_bytes &&
             !max_allocated.compare_exchange_weak(prev_max, current_bytes)) {
      }
      return ptr;
    }

    void do_deallocate(void *p, size_t bytes, size_t alignment) override {
      source->deallocate(p, bytes, alignment);
      allocated_bytes -= bytes;
    }
    [[nodiscard]] bool do_is_equal(const memory_resource &other) const noexcept override {
      return this == &other;
    };

    [[nodiscard]] size_t current_allocated_bytes() const { return allocated_bytes.load(); }
    [[nodiscard]] size_t max_allocated_bytes() const { return max_allocated.load(); }

  private:
    pstd::pmr::memory_resource *source;
    std::atomic<uint64_t> allocated_bytes{0}, max_allocated{0};
  };

  class alignas(SPECULA_L1_CACHE_LINE_SIZE) ScratchBuffer {
  public:
    ScratchBuffer(size_t size = 256) : alloc_size(size) {
      ptr = (char *)Allocator().allocate_bytes(size, align);
    }

    ScratchBuffer(const ScratchBuffer &) = delete;
    ScratchBuffer(ScratchBuffer &&b) noexcept
        : ptr(b.ptr), alloc_size(b.alloc_size), offset(b.offset),
          small_buffers(std::move(b.small_buffers)) {
      b.ptr = nullptr;
      b.alloc_size = b.offset = 0;
    }

    ~ScratchBuffer() {
      reset();
      Allocator().deallocate_bytes(ptr, alloc_size, align);
    }

    ScratchBuffer &operator=(const ScratchBuffer &) = delete;
    ScratchBuffer &operator=(ScratchBuffer &&b) noexcept {
      std::swap(b.ptr, ptr);
      std::swap(b.alloc_size, alloc_size);
      std::swap(b.offset, offset);
      std::swap(b.small_buffers, small_buffers);
      return *this;
    }

    void *alloc(size_t size, size_t align) {
      if ((offset % align) != 0) {
        offset += align - (offset % align);
      }
      if (offset + size > alloc_size) {
        realloc(size);
      }
      void *p = ptr + offset;
      offset += size;
      return p;
    }

    template <typename T, typename... Args>
    typename AllocationTraits<T>::SingleObject alloc(Args &&...args) {
      T *p = (T *)alloc(sizeof(T), alignof(T));
      return new (p) T(std::forward<Args>(args)...);
    }

    template <typename T> typename AllocationTraits<T>::Array alloc(size_t n = 1) {
      using ElementType = typename std::remove_extent_t<T>;
      auto *ret = (ElementType *)alloc(n * sizeof(ElementType), alignof(ElementType));
      for (size_t i = 0; i < n; ++i) {
        new (&ret[i]) ElementType();
      }
      return ret;
    }

    void reset() {
      for (const auto &buf : small_buffers) {
        Allocator().deallocate_bytes(buf.first, buf.second, align);
      }
      small_buffers.clear();
      offset = 0;
    }

  private:
    void realloc(size_t min_size) {
      small_buffers.emplace_back(ptr, alloc_size);
      alloc_size = std::max(2 * min_size, alloc_size + min_size);
      ptr = (char *)Allocator().allocate_bytes(alloc_size, align);
      offset = 0;
    }

    static constexpr int align = SPECULA_L1_CACHE_LINE_SIZE;
    char *ptr = nullptr;
    size_t alloc_size = 0, offset = 0;
    std::list<std::pair<char *, size_t>> small_buffers;
  };

  size_t get_current_rss();
} // namespace specula

#endif // SPECULA_UTIL_MEMORY_HPP
