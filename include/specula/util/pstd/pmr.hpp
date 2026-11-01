/**
 * @file
 * @brief GPU-friendly subset of `std::pmr`: memory resources and a polymorphic allocator.
 *
 * pbrt does not use `<memory_resource>` directly because it is not available in device code
 * (and was missing from some standard libraries when pbrt-v4 was written). This header mirrors
 * pbrt's `pstd::pmr` with only the pieces the renderer needs: the abstract @ref memory_resource,
 * a global new/delete resource, a bump-pointer @ref monotonic_buffer_resource and
 * @ref polymorphic_allocator.
 *
 * @see https://en.cppreference.com/w/cpp/header/memory_resource
 */
#ifndef SPECULA_UTIL_PSTD_PMR_HPP
#define SPECULA_UTIL_PSTD_PMR_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cstddef>

#ifndef NDEBUG
#  include <thread>
#endif

namespace specula::pstd::pmr {
  /** @brief Declared for parity with pbrt/`std::pmr`; not implemented. */
  class synchronized_pool_resource;
  /** @brief Declared for parity with pbrt/`std::pmr`; not implemented. */
  class unsynchronized_pool_resource;

  /**
   * @brief Pool resource tuning options, kept for parity with `std::pmr::pool_options`.
   * @note Nothing in specula consumes this yet (the pool resources are only forward declared).
   */
  struct pool_options {
    /** @brief Maximum number of blocks allocated from upstream at once (0: implementation default).
     */
    size_t max_blocks_per_chunk = 0;
    /** @brief Largest allocation served from a pool (0: implementation default). */
    size_t largest_required_pool_block = 0;
  };

  /**
   * @brief Abstract allocation interface, equivalent to `std::pmr::memory_resource`.
   *
   * Uses the non-virtual interface idiom: callers use allocate() / deallocate() / is_equal(),
   * implementers override the private `do_*` hooks.
   *
   * Deviations from `std`:
   * - allocate() with `bytes == 0` returns `nullptr` without calling do_allocate() (as in pbrt).
   * - deallocate() of `nullptr` is a no-op.
   * - Move construction/assignment are deleted.
   *
   * @see https://en.cppreference.com/w/cpp/memory/memory_resource
   */
  class memory_resource {
  public:
    /** @brief Default alignment used when none is given: `alignof(std::max_align_t)`. */
    static constexpr size_t max_align = alignof(std::max_align_t);

    /** @brief Default constructor. */
    memory_resource() = default;
    /** @brief Copy constructor. */
    memory_resource(const memory_resource &) = default;
    /** @brief Deleted move constructor. */
    memory_resource(memory_resource &&) = delete;

    /** @brief Destructor (defined out of line to anchor the vtable). */
    virtual ~memory_resource();

    /** @brief Copy assignment. */
    memory_resource &operator=(const memory_resource &) = default;
    /** @brief Deleted move assignment. */
    memory_resource &operator=(memory_resource &&) = delete;

    /** @brief Same as is_equal(). */
    bool operator==(const memory_resource &other) const noexcept { return do_is_equal(other); }
    /** @brief Negation of is_equal(). */
    bool operator!=(const memory_resource &other) const noexcept { return !do_is_equal(other); }

    /**
     * @brief Allocates at least `bytes` bytes aligned to `alignment`.
     * @pre `alignment` is a power of two.
     * @return `nullptr` if `bytes == 0`, otherwise whatever do_allocate() returns.
     */
    void *allocate(size_t bytes, size_t alignment = max_align) {
      if (bytes == 0) {
        return nullptr;
      }
      return do_allocate(bytes, alignment);
    }

    /**
     * @brief Returns memory obtained from allocate() with the same `bytes` and `alignment`.
     * @note `p == nullptr` is ignored, which pairs with allocate(0) returning `nullptr`.
     */
    void deallocate(void *p, size_t bytes, size_t alignment = max_align) {
      if (p == nullptr) {
        return;
      }
      do_deallocate(p, bytes, alignment);
    }

    /** @brief True if memory allocated by `*this` may be deallocated by `other` and vice versa. */
    [[nodiscard]] bool is_equal(const memory_resource &other) const noexcept {
      return do_is_equal(other);
    }

    // Pure virtuals are private (NVI idiom), so they follow the public helpers.
  private:
    /** @brief Implementation hook for allocate(); never called with `bytes == 0`. */
    virtual void *do_allocate(size_t bytes, size_t alignment) = 0;
    /** @brief Implementation hook for deallocate(); never called with `p == nullptr`. */
    virtual void do_deallocate(void *p, size_t bytes, size_t alignment) = 0;
    /** @brief Implementation hook for is_equal(). */
    [[nodiscard]] virtual bool do_is_equal(const memory_resource &other) const noexcept = 0;
  };

  /**
   * @brief Process-wide resource backed by the platform's aligned `malloc`/`free`.
   * @warning Lazily created on first call without synchronisation, and never destroyed.
   * @see https://en.cppreference.com/w/cpp/memory/new_delete_resource
   */
  memory_resource *new_delete_resource() noexcept;
  /**
   * @brief Replaces the default resource.
   * @return The previous default resource.
   * @note Unlike `std`, passing `nullptr` does not reset to new_delete_resource(); it stores
   *       `nullptr`. Not thread-safe.
   */
  memory_resource *set_default_resource(memory_resource *r) noexcept;
  /** @brief Current default resource (initially new_delete_resource()). */
  memory_resource *get_default_resource() noexcept;

  /**
   * @brief Bump-pointer arena: fast allocation, deallocation only through release().
   *
   * Requests no larger than `block_size` are carved out of a linked list of upstream blocks;
   * individual deallocate() calls on them are no-ops. Requests larger than `block_size` bypass
   * the arena and go straight to (and back to) `upstream`.
   *
   * `alignas(64)` keeps per-thread arenas on separate cache lines (avoids false sharing).
   * Not thread-safe; in debug builds the constructing thread id is recorded.
   *
   * @warning Unlike `std`, release() does not free the large upstream pass-through
   *          allocations; those must be deallocated explicitly.
   * @see https://en.cppreference.com/w/cpp/memory/monotonic_buffer_resource
   */
  class alignas(64) monotonic_buffer_resource : public memory_resource {
  public:
    /** @brief Uses get_default_resource() as upstream and the default block size. */
    monotonic_buffer_resource() : monotonic_buffer_resource(get_default_resource()) {}
    /** @brief Uses the default block size (64 KiB). */
    explicit monotonic_buffer_resource(memory_resource *upstream) : upstream(upstream) {
#ifndef NDEBUG
      construct_tid = std::this_thread::get_id();
#endif
    }
    /**
     * @brief Uses get_default_resource() as upstream.
     * @param initial_size Block size; unlike `std` it is used for every block, not just the first.
     */
    explicit monotonic_buffer_resource(size_t initial_size)
        : monotonic_buffer_resource(initial_size, get_default_resource()) {}
    /**
     * @brief Fully specified arena.
     * @param block_size Size of each upstream block and threshold above which requests bypass
     *                   the arena.
     */
    monotonic_buffer_resource(size_t block_size, memory_resource *upstream)
        : block_size(block_size), upstream(upstream) {
#ifndef NDEBUG
      construct_tid = std::this_thread::get_id();
#endif
    }
    monotonic_buffer_resource(const monotonic_buffer_resource &) = delete;
    monotonic_buffer_resource(monotonic_buffer_resource &&) = delete;

    /** @brief Calls release(). */
    ~monotonic_buffer_resource() override { release(); }

    monotonic_buffer_resource &operator=(const monotonic_buffer_resource &) = delete;
    monotonic_buffer_resource &operator=(monotonic_buffer_resource &&) = delete;

    /** @brief Returns every arena block to upstream; all pointers into them become dangling. */
    void release() {
      block *b = block_list;
      while (b != nullptr) {
        block *next = b->next;
        free_block(b);
        b = next;
      }
      block_list = nullptr;
      current = nullptr;
      current_pos = 0;
    }

  protected:
    /**
     * @brief Bumps within the current block, starting a new block when the request does not fit.
     *
     * The returned address (not just the offset into the block) is aligned to `align`. A new
     * block is `max(block_size, bytes + align - 1)` bytes so worst-case padding always fits.
     * The remainder of the previous block is abandoned.
     *
     * @pre `align` is a power of two.
     */
    void *do_allocate(size_t bytes, size_t align) override;

    /** @brief No-op for arena memory; forwards pass-through (`bytes > block_size`) blocks upstream.
     */
    void do_deallocate(void *p, size_t bytes, size_t alignment) override {
      if (bytes > block_size) {
        upstream->deallocate(p, bytes);
      }
    }

    /** @brief Arenas only compare equal to themselves. */
    [[nodiscard]] bool do_is_equal(const memory_resource &other) const noexcept override {
      return this == &other;
    }

  private:
    /**
     * @brief Header placed at the start of every upstream block; the payload follows directly.
     *
     * `alignas(std::max_align_t)` makes the payload start max-aligned too.
     */
    struct alignas(std::max_align_t) block {
      /** @brief Start of the payload (`this + 1`). */
      void *ptr;
      /** @brief Payload size in bytes (excludes this header). */
      size_t size;
      /** @brief Previously allocated block (singly linked, newest first). */
      block *next;
    };

    /** @brief Allocates a header + `size`-byte payload from upstream and pushes it on block_list.
     */
    block *allocate_block(size_t size) {
      auto *b = static_cast<block *>(upstream->allocate(sizeof(block) + size, alignof(block)));
      b->ptr = reinterpret_cast<char *>(b) + sizeof(block);
      b->size = size;
      b->next = block_list;
      block_list = b;
      return b;
    }

    /** @brief Returns one block (header + payload) to upstream. */
    void free_block(block *b) { upstream->deallocate(b, sizeof(block) + b->size); }

#ifndef NDEBUG
    /** @brief Thread that constructed the arena (debug only). */
    std::thread::id construct_tid;
#endif

    /** @brief Source of blocks and of pass-through allocations. */
    memory_resource *upstream;
    /** @brief Block payload size and pass-through threshold; default 64 KiB. */
    size_t block_size = 256UL * 256UL;
    /** @brief Block currently being bumped into (`nullptr` before the first allocation). */
    block *current = nullptr;
    /** @brief Next free byte offset within `current`'s payload. */
    size_t current_pos = 0;
    /** @brief Head of the list of all blocks owned by the arena. */
    block *block_list = nullptr;
  };

  /**
   * @brief Allocator that forwards to a @ref memory_resource, equivalent to
   *        `std::pmr::polymorphic_allocator`.
   *
   * This is pbrt's `Allocator` type: containers and scene objects take one so the same code can
   * allocate from host memory, CUDA managed memory or a per-thread arena.
   *
   * Deviations from `std`: the default constructor uses new_delete_resource() rather than
   * get_default_resource(); deallocate() does not pass the alignment on; the move constructor
   * is deleted; the extra object helpers (`allocate_object`, `new_object`, ...) are C++20
   * additions that pbrt also provides.
   *
   * @tparam T Element type for allocate()/deallocate(); defaults to `std::byte` for raw use.
   * @see https://en.cppreference.com/w/cpp/memory/polymorphic_allocator
   */
  template <class T = std::byte> class polymorphic_allocator {
  public:
    /** @brief Allocated element type. */
    using value_type = T;

    /** @brief Uses new_delete_resource(). */
    polymorphic_allocator() noexcept : memory_resource(new_delete_resource()) {}
    /** @brief Uses `r`; implicit so a resource pointer can be passed wherever an allocator is
     * expected. */
    polymorphic_allocator(memory_resource *r) : memory_resource(r) {}
    /** @brief Copy constructor (shares the resource). */
    polymorphic_allocator(const polymorphic_allocator &other) = default;
    /** @brief Rebinding copy constructor (shares the resource). */
    template <class U>
    polymorphic_allocator(const polymorphic_allocator<U> &other) noexcept
        : memory_resource(other.resource()) {}
    /** @brief Deleted move constructor. */
    polymorphic_allocator(polymorphic_allocator &&) = delete;

    /** @brief Destructor. */
    ~polymorphic_allocator() = default;

    /** @brief Deleted copy assignment: the resource is fixed for the allocator's lifetime. */
    polymorphic_allocator &operator=(const polymorphic_allocator &other) = delete;
    /** @brief Deleted move assignment. */
    polymorphic_allocator &operator=(polymorphic_allocator &&) = delete;

    /** @brief Underlying memory resource. */
    [[nodiscard]] memory_resource *resource() const { return memory_resource; }

    /** @brief True if both use the same resource pointer (not `memory_resource::is_equal`). */
    template <class U> bool operator==(const polymorphic_allocator<U> &rhs) noexcept {
      return memory_resource == rhs.memory_resource;
    }
    /** @brief Negation of operator==(). */
    template <class U> bool operator!=(const polymorphic_allocator<U> &rhs) noexcept {
      return memory_resource != rhs.memory_resource;
    }

    // Typed allocation
    /** @brief Uninitialised storage for `n` objects of type `T`. */
    [[nodiscard]] value_type *allocate(size_t n) {
      return static_cast<value_type *>(
          memory_resource->allocate(n * sizeof(value_type), alignof(value_type)));
    }
    /**
     * @brief Frees storage from allocate().
     * @note Passes the default alignment, not `alignof(T)`, to the resource (as pbrt does).
     */
    void deallocate(value_type *p, size_t n) {
      memory_resource->deallocate(p, n * sizeof(value_type));
    }

    // Raw and per-object allocation (C++20 additions)
    /** @brief Raw storage, independent of `T`. */
    void *allocate_bytes(size_t nbytes, size_t alignment = alignof(max_align_t)) {
      return memory_resource->allocate(nbytes, alignment);
    }
    /** @brief Frees storage from allocate_bytes(). */
    void deallocate_bytes(void *p, size_t nbytes, size_t alignment = alignof(max_align_t)) {
      memory_resource->deallocate(p, nbytes, alignment);
    }

    /** @brief Uninitialised storage for `n` objects of type `U`. */
    template <class U> U *allocate_object(size_t n = 1) {
      return static_cast<U *>(allocate_bytes(n * sizeof(U), alignof(U)));
    }
    /** @brief Frees storage from allocate_object(). */
    template <class U> void deallocate_object(U *p, size_t n = 1) {
      deallocate_bytes(p, n * sizeof(U), alignof(U));
    }

    /**
     * @brief Allocates and constructs a single `U`.
     * @warning If the constructor throws, the storage is leaked (`std` frees it).
     */
    template <class U, class... Args> U *new_object(Args &&...args) {
      U *p = allocate_object<U>();
      construct(p, std::forward<Args>(args)...);
      return p;
    }
    /** @brief Destroys and frees an object from new_object(). */
    template <class U> void delete_object(U *p) {
      destroy(p);
      deallocate_object(p);
    }

    /** @brief Placement-constructs `U` at `p`; no uses-allocator construction, unlike `std`. */
    template <class U, class... Args> void construct(U *p, Args &&...args) {
      ::new ((void *)p) U(std::forward<Args>(args)...);
    }
    /** @brief Calls `p->~U()`; does not free. */
    template <class U> void destroy(U *p) { p->~U(); }

  private:
    /** @brief Backing resource; never owned. */
    memory_resource *memory_resource;
  };
} // namespace specula::pstd::pmr

#endif // SPECULA_UTIL_PSTD_PMR_HPP
