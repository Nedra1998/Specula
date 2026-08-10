#ifndef SPECULA_UTIL_PSTD_PMR_HPP
#define SPECULA_UTIL_PSTD_PMR_HPP

#include <cstddef>

#ifndef NDEBUG
#  include <thread>
#endif

namespace specula::pstd::pmr {
  class synchronized_pool_resource;
  class unsynchronized_pool_resource;

  class memory_resource {
  public:
    memory_resource() = default;
    memory_resource(const memory_resource &) = default;
    memory_resource(memory_resource &&) = delete;
    memory_resource &operator=(const memory_resource &) = default;
    memory_resource &operator=(memory_resource &&) = delete;

    static constexpr size_t max_align = alignof(std::max_align_t);
    virtual ~memory_resource();

    void *allocate(size_t bytes, size_t alignment = max_align) {
      if (bytes == 0) {
        return nullptr;
      }
      return do_allocate(bytes, alignment);
    }

    void deallocate(void *p, size_t bytes, size_t alignment = max_align) {
      if (p == nullptr) {
        return;
      }
      do_deallocate(p, bytes, alignment);
    }

    [[nodiscard]] bool is_equal(const memory_resource &other) const noexcept {
      return do_is_equal(other);
    }

    bool operator==(const memory_resource &other) const noexcept { return do_is_equal(other); }
    bool operator!=(const memory_resource &other) const noexcept { return !do_is_equal(other); }

  private:
    virtual void *do_allocate(size_t bytes, size_t alignment) = 0;
    virtual void do_deallocate(void *p, size_t bytes, size_t alignment) = 0;
    [[nodiscard]] virtual bool do_is_equal(const memory_resource &other) const noexcept = 0;
  };

  struct pool_options {
    size_t max_blocks_per_chunk = 0;
    size_t largest_required_pool_block = 0;
  };

  memory_resource *new_delete_resource() noexcept;
  memory_resource *set_default_resource(memory_resource *r) noexcept;
  memory_resource *get_default_resource() noexcept;

  class alignas(64) monotonic_buffer_resource : public memory_resource {
  public:
    explicit monotonic_buffer_resource(memory_resource *upstream) : upstream(upstream) {
#ifndef NDEBUG
      construct_tid = std::this_thread::get_id();
#endif
    }

    monotonic_buffer_resource(size_t block_size, memory_resource *upstream)
        : block_size(block_size), upstream(upstream) {
#ifndef NDEBUG
      construct_tid = std::this_thread::get_id();
#endif
    }

    monotonic_buffer_resource() : monotonic_buffer_resource(get_default_resource()) {}
    explicit monotonic_buffer_resource(size_t initial_size)
        : monotonic_buffer_resource(initial_size, get_default_resource()) {}

    monotonic_buffer_resource(const monotonic_buffer_resource &) = delete;
    monotonic_buffer_resource(monotonic_buffer_resource &&) = delete;

    ~monotonic_buffer_resource() override { release(); }

    monotonic_buffer_resource &operator=(const monotonic_buffer_resource &) = delete;
    monotonic_buffer_resource &operator=(monotonic_buffer_resource &&) = delete;

    void release() {
      block *b = block_list;
      while (b != nullptr) {
        block *next = b->next;
        free_block(b);
        b = next;
      }
      block_list = nullptr;
      current = nullptr;
    }

  protected:
    void *do_allocate(size_t bytes, size_t align) override;

    void do_deallocate(void *p, size_t bytes, size_t alignment) override {
      if (bytes > block_size) {
        upstream->deallocate(p, bytes);
      }
    }

    [[nodiscard]] bool do_is_equal(const memory_resource &other) const noexcept override {
      return this == &other;
    }

  private:
    struct block {
      void *ptr;
      size_t size;
      block *next;
    };

    block *allocate_block(size_t size) {
      auto *b = static_cast<block *>(upstream->allocate(sizeof(block) + size, alignof(block)));
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
    size_t block_size = 256UL * 256UL;
    block *current = nullptr;
    size_t current_pos = 0;
    block *block_list = nullptr;
  };

  template <class T = std::byte> class polymorphic_allocator {
  public:
    using value_type = T;

    polymorphic_allocator() noexcept : memory_resource(new_delete_resource()) {}
    polymorphic_allocator(memory_resource *r) : memory_resource(r) {}
    polymorphic_allocator(const polymorphic_allocator &other) = default;
    template <class U>
    polymorphic_allocator(const polymorphic_allocator<U> &other) noexcept
        : memory_resource(other.memory_resource) {}
    polymorphic_allocator(polymorphic_allocator &&) = delete;

    polymorphic_allocator &operator=(const polymorphic_allocator &other) = delete;
    polymorphic_allocator &operator=(polymorphic_allocator &&) = delete;

    ~polymorphic_allocator() = default;

    [[nodiscard]] value_type *allocate(size_t n) {
      return static_cast<value_type *>(
          memory_resource->allocate(n * sizeof(value_type), alignof(value_type)));
    }
    void deallocate(value_type *p, size_t n) {
      memory_resource->deallocate(p, n * sizeof(value_type));
    }

    void *allocate_bytes(size_t nbytes, size_t alignment = alignof(max_align_t)) {
      return memory_resource->allocate(nbytes, alignment);
    }
    void deallocate_bytes(void *p, size_t nbytes, size_t alignment = alignof(max_align_t)) {
      memory_resource->deallocate(p, nbytes, alignment);
    }

    template <class U> U *allocate_object(size_t n = 0) {
      return static_cast<U *>(allocate_bytes(n * sizeof(U), alignof(U)));
    }
    template <class U> void deallocate_object(T *p, size_t n = 1) {
      deallocate_bytes(p, n * sizeof(U), alignof(U));
    }

    template <class U, class... Args> U *new_object(Args &&...args) {
      U *p = allocate_object9<U>();
      construct(p, std::forward<Args>(args)...);
      return p;
    }
    template <class U> void delete_object(U *p) {
      destroy(p);
      deallocate_object(p);
    }

    template <class U, class... Args> void construct(U *p, Args &&...args) {
      ::new ((void *)p) U(std::forward<Args>(args)...);
    }
    template <class U> void destroy(U *p) { p->~U(); }

    [[nodiscard]] memory_resource *resource() const { return memory_resource; }

    template <class U> bool operator==(const polymorphic_allocator<U> &rhs) noexcept {
      return memory_resource == rhs.memory_resource;
    }
    template <class U> bool operator!=(const polymorphic_allocator<U> &rhs) noexcept {
      return memory_resource != rhs.memory_resource;
    }

  private:
    memory_resource *memory_resource;
  };
} // namespace specula::pstd::pmr

#endif // SPECULA_UTIL_PSTD_PMR_HPP
