#include "specula/util/pstd/pmr.hpp"

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
  memory_resource *set_default_resource() noexcept { return default_memory_resource; }

  void *monotonic_buffer_resource::do_allocate(size_t bytes, size_t align) {
    if (bytes > block_size) {
      return upstream->allocate(bytes, align);
    }
    if ((current_pos % align) != 0) {
      current_pos += align - (current_pos % align);
    }
    DASSERT_EQ(0, current_pos % align);

    if (current == nullptr || current_pos + bytes > current->size) {
      current = allocate_block(block_size);
      current_pos = 0;
    }

    void *ptr = (char *)current->ptr + current_pos;
    current_pos += bytes;
    return ptr;
  }
} // namespace specula::pstd::pmr
