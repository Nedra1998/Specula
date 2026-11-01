#include "specula/util/pstd/pmr.hpp"

#include <algorithm>
#include <cstdint>

#include "specula/util/check.hpp"

namespace specula::pstd::pmr {
  memory_resource::~memory_resource() = default;

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

  static NewDeleteResource *ndr;

  memory_resource *new_delete_resource() noexcept {
    if (ndr == nullptr) {
      ndr = new NewDeleteResource;
    }
    return ndr;
  }

  static memory_resource *default_memory_resource = new_delete_resource();

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
