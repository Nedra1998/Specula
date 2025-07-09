/**
 * @file pmr.hpp
 * @brief Polymoric Memory Resource (PMR) classes
 *
 * This file provides a portable implementation of the C++17 Polymorphic Memory Resource classes,
 * which are used to provide a common interface for memory allocation and deallocation. The classes
 * in this file are designed to replace the standard library classes, and are usable both on the CPU
 * and GPU platforms.
 */

#ifndef SPECULA_UTIL_PSTD_PMR_HPP_
#define SPECULA_UTIL_PSTD_PMR_HPP_

#include <cstddef>
#include <thread>

/**
 * @brief Namespace for Polymorphic Memory Resource (PMR) classes, derived from the `std::pmr`
 * namespace.
 *
 * This namespace contains re-implementations of the C++17 Polymorphic Memory Resource classes,
 * which are used to provide a common interface for memory allocation and deallocation. The classes
 * in this namespace are designed to replace the standard library classes, and are usable both on
 * the CPU and GPU.
 *
 * @see https://en.cppreference.com/w/cpp/header/memory_resource
 */
namespace specula::pstd::pmr {
  /**
   * @class memory_resource
   * @brief An abstract interface for classes that encapsulate memory resources.
   *
   * @see https://en.cppreference.com/w/cpp/memory/memory_resource
   */
  class memory_resource {
  protected:
    static constexpr size_t max_align = alignof(std::max_align_t);

  public:
    virtual ~memory_resource();
    inline void *allocate(size_t bytes, size_t alignment = max_align) {
      if (bytes == 0)
        return nullptr;
      return do_allocate(bytes, alignment);
    }

    inline void deallocate(void *p, size_t bytes, size_t alignment = max_align) {
      if (!p)
        return;
      return do_deallocate(p, bytes, alignment);
    }

    inline bool is_equal(const memory_resource &other) const noexcept { return do_is_equal(other); }

    inline bool operator==(const memory_resource &other) const noexcept { return is_equal(other); }
    inline bool operator!=(const memory_resource &other) const noexcept { return !is_equal(other); }

  private:
    virtual void *do_allocate(size_t bytes, size_t alignment) = 0;
    virtual void do_deallocate(void *p, size_t bytes, size_t alignment) = 0;
    virtual bool do_is_equal(const memory_resource &other) const noexcept = 0;
  };

  /**
   * @class pool_options
   * @brief A set of constructor options for pool resources.
   *
   * @see https://en.cppreference.com/w/cpp/memory/pool_options
   */
  struct pool_options {
    /// @brief The maximum number of blocks that will be allocated at once from the upstream
    /// `memory_resource` to replenish the pool.
    size_t max_blocks_per_chunk = 0;

    /// @brief The largest allocation size that is required to be fulfilled using the pooling
    /// mechanism.
    size_t largest_required_pool_block = 0;
  };

  class synchronized_pool_resource;
  class unsynchronized_pool_resource;

  /**
   * @brief Returns a static program-wide `memory_resource` that uses the global operator new and
   * operator delete to allocate and deallocate memory.
   *
   * @return A pointer to the global new/delete memory resource.
   * @see https://en.cppreference.com/w/cpp/memory/new_delete_resource
   */
  memory_resource *new_delete_resource() noexcept;

  /**
   * @brief Sets the default `memory_resource`.
   *
   * @param r The memory_resource to be used as the default.
   * @return The previous default memory_resource.
   * @see https://en.cppreference.com/w/cpp/memory/set_default_resource
   */
  memory_resource *set_default_resource(memory_resource *r) noexcept;
  /**
   * @brief Get the current default `memory_resource`.
   *
   * @return The current default memory_resource.
   * @see https://en.cppreference.com/w/cpp/memory/get_default_resource
   */
  memory_resource *get_default_resource() noexcept;

  /**
   * @class monotonic_buffer_resource
   * @brief A special-purpose `memory_resource` that releases the allocated memory only when the
   * resource is destroyed.
   *
   * @see https://en.cppreference.com/w/cpp/memory/monotonic_buffer_resource
   */
  class alignas(64) monotonic_buffer_resource : public memory_resource {
  public:
    explicit monotonic_buffer_resource(memory_resource *upstream) : upstream(upstream) {
#ifndef NDEBUG
      construct_tid = std::this_thread::get_id();
#endif
    }

    monotonic_buffer_resource(size_t block_size, memory_resource *upstream)
        : upstream(upstream), block_size(block_size) {
#ifndef NDEBUG
      construct_tid = std::this_thread::get_id();
#endif
    }

    monotonic_buffer_resource() : monotonic_buffer_resource(get_default_resource()) {}
    explicit monotonic_buffer_resource(size_t initial_size)
        : monotonic_buffer_resource(initial_size, get_default_resource()) {}

    monotonic_buffer_resource(const monotonic_buffer_resource &) = delete;

    ~monotonic_buffer_resource() { release(); }

    monotonic_buffer_resource &operator=(const monotonic_buffer_resource &) = delete;

    void release() {
      block *b = block_list;
      while (b) {
        block *next = b->next;
        free_block(b);
        b = next;
      }
      block_list = nullptr;
      current = nullptr;
    }

    memory_resource *upstream_resource() const { return upstream; }

  protected:
    void *do_allocate(size_t bytes, size_t align) override;

    void do_deallocate(void *p, size_t bytes, size_t alignment) override {
      if (bytes > block_size)
        upstream->deallocate(p, bytes);
    }

    bool do_is_equal(const memory_resource &other) const noexcept override {
      return this == &other;
    }

  private:
    struct block {
      void *ptr;
      size_t size;
      block *next;
    };

    block *allocate_block(size_t size) {
      block *b = static_cast<block *>(upstream->allocate(sizeof(block) + size, alignof(block)));
      b->ptr = reinterpret_cast<char *>(b) + sizeof(block);
      b->size = size;
      b->next = block_list;
      block_list = b;
      return b;
    }

    void free_block(block *b) { upstream->deallocate(b, sizeof(block) + b->size); }

#ifndef NDEBUG
    std::thread::id construct_tid;
#endif

    memory_resource *upstream;
    size_t block_size = 256 * 1024;
    block *current = nullptr;
    size_t current_pos = 0;
    block *block_list = nullptr;
  };

  /**
   * @brief An allocator that supports run-time polymorphism based on the `memory_resource` it is
   * constructed with.
   *
   * @see https://en.cppreference.com/w/cpp/memory/polymorphic_allocator
   */
  template <typename T = std::byte> class polymorphic_allocator {
  public:
    using value_type = T;

    polymorphic_allocator() noexcept { memory_resource_ = get_default_resource(); }
    polymorphic_allocator(memory_resource *r) : memory_resource_(r) {}
    polymorphic_allocator(const polymorphic_allocator &other) = default;
    template <class U>
    polymorphic_allocator(const polymorphic_allocator<U> &other) noexcept
        : memory_resource_(other.resource()) {}

    polymorphic_allocator &operator=(const polymorphic_allocator &rhs) = delete;

    [[nodiscard]] T *allocate(size_t n) {
      return static_cast<T *>(resource()->allocate(n * sizeof(T), alignof(T)));
    }
    void deallocate(T *p, size_t n) { resource()->deallocate(p, n * sizeof(T)); }
    void *allocate_bytes(size_t nbytes, size_t alignment = alignof(max_align_t)) {
      return resource()->allocate(nbytes, alignment);
    }
    void deallocate_bytes(void *p, size_t nbytes, size_t alignment = alignof(std::max_align_t)) {
      return resource()->deallocate(p, nbytes, alignment);
    }

    template <class U> U *allocate_object(size_t n = 1) {
      return static_cast<U *>(allocate_bytes(n * sizeof(U), alignof(U)));
    }

    template <class U> void deallocate_object(U *p, size_t n = 1) {
      deallocate_bytes(p, n * sizeof(U), alignof(U));
    }
    template <class U, class... Args> U *new_object(Args &&...args) {
      U *p = allocate_object<U>();
      construct(p, std::forward<Args>(args)...);
      return p;
    }
    template <class U> void delete_object(U *p) {
      destroy(p);
      dallocate_object(p);
    }
    template <class U, class... Args> void construct(U *p, Args &&...args) {
      ::new ((void *)p) U(std::forward<Args>(args)...);
    }
    template <class U> void destroy(U *p) { p->~U(); }

    memory_resource *resource() const { return memory_resource_; }

    template <class U> bool operator==(const polymorphic_allocator<U> &rhs) noexcept {
      return resource() == rhs.resource();
    }

    template <class U> bool operator!=(const polymorphic_allocator<U> &rhs) noexcept {
      return resource() != rhs.resource();
    }

  private:
    memory_resource *memory_resource_;
  };
} // namespace specula::pstd::pmr

#endif // SPECULA_UTIL_PSTD_PMR_HPP_
