/**
 * @file
 * @brief Out-of-line parts of pstd::pmr: the new/delete resource, the default-resource global
 *        and monotonic_buffer_resource::do_allocate().
 */
#include "specula/util/pstd/pmr.hpp"

#include <algorithm>
#include <cstdint>

#include "specula/util/check.hpp"

namespace specula::pstd::pmr {
  namespace {
    /**
     * @brief Concrete resource behind new_delete_resource().
     *
     * Despite the name it uses the platform aligned allocator rather than `operator new`:
     * `_aligned_malloc` on Windows, `posix_memalign` where available (plain `malloc` for
     * alignments below `sizeof(void *)`, which `posix_memalign` rejects), `memalign` otherwise.
     */
    class NewDeleteResource : public memory_resource {
      void *do_allocate(size_t bytes, size_t alignment) override {
#if defined(SPECULA_HAVE__ALIGNED_MALLOC)
        return _aligned_malloc(size, alignment);
#elif defined(SPECULA_HAVE_POSIX_MEMALIGN)
        void *ptr = nullptr;
        if (alignment < sizeof(void *)) {
          return malloc(bytes);
        }
        if (posix_memalign(&ptr, alignment, bytes) != 0) {
          ptr = nullptr;
        }
        return ptr;
#else
        return memalign(alignemnt, bytes);
#endif
      }

      void do_deallocate(void *ptr, size_t bytes, size_t alignment) override {
        if (ptr == nullptr) {
          return;
        }
#if defined(SPECULA_HAVE__ALIGNED_MALLOC)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
      }

      [[nodiscard]] bool do_is_equal(const memory_resource &other) const noexcept override {
        return this == &other;
      }
    };

    /** @brief Lazily created singleton returned by new_delete_resource(); intentionally leaked. */
    NewDeleteResource *ndr;

    /**
     * @brief Backing store for get/set_default_resource().
     *
     * Dynamically initialised, so another TU's static initialiser calling get_default_resource()
     * may observe `nullptr`. `ndr` is zero-initialised before this runs, so the call is safe.
     */
    memory_resource *default_memory_resource = new_delete_resource();
  } // namespace

  memory_resource::~memory_resource() = default;

  memory_resource *new_delete_resource() noexcept {
    if (ndr == nullptr) {
      ndr = new NewDeleteResource;
    }
    return ndr;
  }

  memory_resource *set_default_resource(memory_resource *r) noexcept {
    memory_resource *orig = default_memory_resource;
    default_memory_resource = r;
    return orig;
  }
  memory_resource *get_default_resource() noexcept { return default_memory_resource; }

  void *monotonic_buffer_resource::do_allocate(size_t bytes, size_t align) {
    if (bytes > block_size) {
      return upstream->allocate(bytes, align);
    }

    // Align the absolute address, not just the offset: the block's data
    // pointer itself is only guaranteed to be aligned to alignof(block).
    auto aligned_offset = [align](const block *b, size_t pos) -> size_t {
      const auto base = reinterpret_cast<uintptr_t>(b->ptr);
      const uintptr_t addr = (base + pos + align - 1) & ~(uintptr_t(align) - 1);
      return addr - base;
    };

    size_t offset = 0;
    if (current != nullptr) {
      offset = aligned_offset(current, current_pos);
    }
    if (current == nullptr || offset + bytes > current->size) {
      // Reserve enough room for worst-case alignment padding in the new block.
      current = allocate_block(std::max(block_size, bytes + align - 1));
      offset = aligned_offset(current, 0);
    }
    DASSERT_EQ(0, (reinterpret_cast<uintptr_t>(current->ptr) + offset) % align);

    void *ptr = static_cast<char *>(current->ptr) + offset;
    current_pos = offset + bytes;
    return ptr;
  }
} // namespace specula::pstd::pmr
