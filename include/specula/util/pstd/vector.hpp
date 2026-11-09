/**
 * @file
 * @brief `pstd::vector`, an allocator-aware dynamic array ported from pbrt-v4's `pstd::vector`.
 *
 * pbrt cannot use `std::vector` in code shared with the GPU, so it ships its own minimal vector
 * whose storage comes from a `pmr::polymorphic_allocator` (e.g. CUDA managed memory). Only the
 * read-only accessors are `SPECULA_CPU_GPU`; anything that allocates or destroys is host-only.
 *
 * @see https://en.cppreference.com/w/cpp/container/vector
 */

#ifndef SPECULA_UTIL_PSTD_VECTOR_HPP
#define SPECULA_UTIL_PSTD_VECTOR_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cstdlib>
#include <initializer_list>
#include <iterator>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/pstd/pmr.hpp"
#include "specula/util/pstd/utility.hpp"

namespace specula::pstd {
  /**
   * @brief Contiguous dynamic array whose storage is obtained from `Allocator`.
   *
   * Iterators are raw pointers. Growth starts at a capacity of 4 and doubles thereafter.
   *
   * Differences from `std::vector`:
   * - allocators are never propagated on copy/move assignment or swap; when the allocators compare
   *   unequal, elements are copied or moved one by one instead of stealing the buffer;
   * - `insert` only supports appending at `end()`;
   * - no `erase`, `emplace`, `at`, `shrink_to_fit` or comparison operators.
   *
   * @see pmr::polymorphic_allocator
   */
  template <typename T, class Allocator = pmr::polymorphic_allocator<T>> class vector {
  public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = value_type *;
    using const_iterator = const value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    /** @brief Constructs an empty vector with no storage (`data() == nullptr`). */
    vector(const Allocator &alloc = {}) : alloc(alloc) {}

    /** @brief Constructs `count` copies of `value`. */
    vector(size_type count, const T &value, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(count);
      for (size_type i = 0; i < count; ++i) {
        this->alloc.template construct<value_type>(ptr + i, value);
      }
      n_stored = count;
    }

    /**
     * @brief Constructs `count` copies of a value-initialised `T{}`.
     * @note Unlike `std::vector`, this is not `explicit`, and elements are copy-constructed from a
     * single `T{}` rather than each being value-initialised in place.
     */
    vector(size_type count, const Allocator &alloc = {}) : vector(count, T{}, alloc) {}

    /**
     * @brief Copies the range `[first, last)`.
     * @pre `last - first` must be valid, i.e. `InputIt` must in practice be random access.
     * @warning Unconstrained, unlike `std::vector`: `vector<int>(3, 5)` selects this overload with
     * `InputIt = int` instead of the `(count, value)` constructor.
     */
    template <class InputIt>
    vector(InputIt first, InputIt last, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(last - first);
      size_type i = 0;
      for (InputIt iter = first; iter != last; ++iter, ++i) {
        this->alloc.template construct<value_type>(ptr + i, *iter);
      }
      n_stored = n_alloc;
    }

    /** @brief Copies the elements of `init`. */
    vector(std::initializer_list<value_type> init, const Allocator &alloc = {})
        : vector(init.begin(), init.end(), alloc) {}

    /**
     * @brief Deep copy of `other` into storage from `alloc`.
     * @note `alloc` defaults to `Allocator{}`, not `other.get_allocator()` as in `std::vector`.
     */
    vector(const vector &other, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(other.size());
      for (size_type i = 0; i < other.size(); ++i) {
        this->alloc.template construct<value_type>(ptr + i, other[i]);
      }
      n_stored = other.size();
    }

    /** @brief Steals `other`'s buffer and allocator, leaving `other` empty. */
    vector(vector &&other) noexcept
        : alloc(other.alloc), ptr(other.ptr), n_alloc(other.n_alloc), n_stored(other.n_stored) {
      other.n_stored = other.n_alloc = 0;
      other.ptr = nullptr;
    }

    /**
     * @brief Steals `other`'s buffer if `alloc == other.get_allocator()`, otherwise copies it.
     * @note In the unequal-allocator case the elements are copied (not moved), and `other` keeps
     * its contents.
     */
    vector(vector &&other, Allocator alloc) : alloc(alloc) {
      if (alloc == other.alloc) {
        pstd::swap(ptr, other.ptr);
        pstd::swap(n_alloc, other.n_alloc);
        pstd::swap(n_stored, other.n_stored);
      } else {
        reserve(other.size());
        for (size_type i = 0; i < other.size(); ++i) {
          this->alloc.template construct<value_type>(ptr + i, other[i]);
        }
        n_stored = other.size();
      }
    }

    /** @brief Destroys all elements and releases the buffer. */
    ~vector() {
      clear();
      alloc.deallocate_object(ptr, n_alloc);
    }

    /** @brief Deep copy; keeps this vector's allocator. */
    vector &operator=(const vector &other) {
      if (this == &other) {
        return *this;
      }

      clear();
      reserve(other.size());
      for (size_type i = 0; i < other.size(); ++i) {
        alloc.template construct<value_type>(ptr + i, other[i]);
      }
      n_stored = other.size();
      return *this;
    }

    /**
     * @brief Swaps buffers when the allocators compare equal, otherwise move-constructs each
     * element into this vector's storage.
     * @warning Marked `noexcept` although the unequal-allocator path allocates.
     */
    vector &operator=(vector &&other) noexcept {
      if (this == &other) {
        return *this;
      }

      if (alloc == other.alloc) {
        pstd::swap(ptr, other.ptr);
        pstd::swap(n_alloc, other.n_alloc);
        pstd::swap(n_stored, other.n_stored);
      } else {
        clear();
        reserve(other.size());
        for (size_type i = 0; i < other.size(); ++i) {
          alloc.template construct<value_type>(ptr + i, std::move(other[i]));
        }
        n_stored = other.size();
      }

      return *this;
    }

    /**
     * @brief Replaces the contents with the elements of `init`.
     * @todo Copy-*assigns* into destroyed/unconstructed storage instead of constructing; UB for
     * non-trivial `T`.
     */
    vector &operator=(const std::initializer_list<value_type> &init) {
      reserve(init.size());
      clear();
      iterator iter = begin();
      for (const auto &value : init) {
        *iter = value;
        ++iter;
      }
      n_stored = init.size();
      return *this;
    }

    /** @brief Returns a copy of the allocator. */
    allocator_type get_allocator() const { return alloc; }

    /** @brief Iterator to the first element. */
    SPECULA_CPU_GPU iterator begin() { return ptr; }
    /** @brief Iterator to the first element. */
    SPECULA_CPU_GPU const_iterator begin() const { return ptr; }
    /** @brief Iterator one past the last element. */
    SPECULA_CPU_GPU iterator end() { return ptr + n_stored; }
    /** @brief Iterator one past the last element. */
    SPECULA_CPU_GPU const_iterator end() const { return ptr + n_stored; }
    /** @brief Const iterator to the first element. */
    SPECULA_CPU_GPU const_iterator cbegin() const { return ptr; }
    /** @brief Const iterator one past the last element. */
    SPECULA_CPU_GPU const_iterator cend() const { return ptr + n_stored; }
    /** @brief Reverse iterator to the last element. */
    SPECULA_CPU_GPU reverse_iterator rbegin() { return reverse_iterator(end()); }
    /** @brief Reverse iterator to the last element. */
    SPECULA_CPU_GPU const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    /** @brief Reverse iterator one before the first element. */
    SPECULA_CPU_GPU reverse_iterator rend() { return reverse_iterator(begin()); }
    /** @brief Reverse iterator one before the first element. */
    SPECULA_CPU_GPU const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    /** @brief Number of constructed elements. */
    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return n_stored; }
    /** @brief Whether `size() == 0`. */
    SPECULA_CPU_GPU [[nodiscard]] bool empty() const { return n_stored == 0; }
    /** @brief Largest representable size, `size_type(-1)`; not tied to the allocator. */
    SPECULA_CPU_GPU [[nodiscard]] size_type max_size() const { return (size_type)-1; }
    /** @brief Number of elements the current buffer can hold without reallocating. */
    SPECULA_CPU_GPU [[nodiscard]] size_type capacity() const { return n_alloc; }

    /** @brief Unchecked in release builds; `DASSERT`s `index < size()` in debug builds. */
    SPECULA_CPU_GPU reference operator[](size_type index) {
      DASSERT_LT(index, size());
      return ptr[index];
    }
    /** @brief Unchecked in release builds; `DASSERT`s `index < size()` in debug builds. */
    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      DASSERT_LT(index, size());
      return ptr[index];
    }

    /** @brief First element. @pre `!empty()` (not checked). */
    SPECULA_CPU_GPU reference front() { return ptr[0]; }
    /** @brief First element. @pre `!empty()` (not checked). */
    SPECULA_CPU_GPU const_reference front() const { return ptr[0]; }
    /** @brief Last element. @pre `!empty()` (not checked). */
    SPECULA_CPU_GPU reference back() { return ptr[n_stored - 1]; }
    /** @brief Last element. @pre `!empty()` (not checked). */
    SPECULA_CPU_GPU const_reference back() const { return ptr[n_stored - 1]; }

    /** @brief Pointer to the buffer; `nullptr` if no storage has been allocated. */
    SPECULA_CPU_GPU pointer data() { return ptr; }
    /** @brief Pointer to the buffer; `nullptr` if no storage has been allocated. */
    SPECULA_CPU_GPU const_pointer data() const { return ptr; }

    /** @brief Replaces the contents with `count` copies of `value`. */
    void assign(size_type count, const value_type &value) {
      clear();
      reserve(count);
      for (size_type i = 0; i < count; ++i) {
        push_back(value);
      }
    }

    /**
     * @brief Grows the buffer to exactly `n` elements if `capacity() < n`, moving the existing
     * elements; never shrinks.
     */
    void reserve(size_type n) {
      if (n_alloc >= n) {
        return;
      }

      value_type *ra = alloc.template allocate_object<T>(n);
      for (size_type i = 0; i < n_stored; ++i) {
        alloc.template construct<T>(ra + i, std::move(begin()[i]));
        alloc.destroy(begin() + i);
      }

      alloc.deallocate_object(ptr, n_alloc);
      n_alloc = n;
      ptr = ra;
    }

    /** @brief Destroys all elements; capacity is kept. */
    void clear() {
      for (size_type i = 0; i < n_stored; ++i) {
        alloc.destroy(&ptr[i]);
      }
      n_stored = 0;
    }

    /**
     * @brief Inserts `[first, last)` before `pos`; only `pos == end()` (append) is supported.
     * @return Iterator to the first inserted element.
     * @todo Any other `pos` logs a critical error and then falls off the end without returning
     * (UB); pbrt uses `LOG_FATAL` here, which aborts.
     */
    template <class InputIt> iterator insert(const_iterator pos, InputIt first, InputIt last) {
      if (pos == end()) {
        size_t first_offset = size();
        for (auto iter = first; iter != last; ++iter) {
          push_back(*iter);
        }
        return begin() + first_offset;
      }
      LOG_CRITICAL("Unimplemented");
      std::quick_exit(1);
    }

    /** @brief Constructs an element in place at the end. Returns `void`, unlike C++17 `std`. */
    template <class... Args> void emplace_back(Args &&...args) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, std::forward<Args>(args)...);
      ++n_stored;
    }

    /**
     * @brief Appends a copy of `value`.
     * @warning `value` must not alias an element of this vector: if a reallocation happens it is
     * read after the old buffer has been freed.
     */
    void push_back(const T &value) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, value);
      ++n_stored;
    }

    /** @brief Appends `value` by move. */
    void push_back(T &&value) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, std::move(value));
      ++n_stored;
    }

    /** @brief Destroys the last element. @pre `!empty()` (`DASSERT`ed). */
    void pop_back() {
      DASSERT(!empty());
      alloc.destroy(ptr + n_stored - 1);
      --n_stored;
    }

    /**
     * @brief Shrinks by destroying trailing elements, or grows with value-initialised ones.
     * @note `resize(0)` on a non-empty vector also releases the buffer (pbrt behaviour); `clear()`
     * keeps it.
     */
    void resize(size_type n) {
      if (n < size()) {
        for (size_t i = n; i < size(); ++i) {
          alloc.destroy(ptr + i);
        }
        if (n == 0) {
          alloc.deallocate_object(ptr, n_alloc);
          ptr = nullptr;
          n_alloc = 0;
        }
      } else if (n > size()) {
        reserve(n);
        for (size_t i = size(); i < n; ++i) {
          alloc.construct(ptr + i);
        }
      }
      n_stored = n;
    }

    /**
     * @brief Swaps contents. Buffers are exchanged when the allocators compare equal; otherwise
     * elements are moved through a temporary and each vector keeps its own allocator.
     * @warning Marked `noexcept` although the unequal-allocator path allocates.
     */
    void swap(vector &other) noexcept {
      if (alloc == other.alloc) {
        pstd::swap(ptr, other.ptr);
        pstd::swap(n_alloc, other.n_alloc);
        pstd::swap(n_stored, other.n_stored);
      } else {
        vector tmp(std::move(*this));
        *this = std::move(other);
        other = std::move(tmp);
      }
    }

  private:
    /** @brief Source of the buffer; never replaced after construction. */
    Allocator alloc;
    /** @brief Buffer of `n_alloc` slots, the first `n_stored` constructed; may be `nullptr`. */
    value_type *ptr = nullptr;
    /** @brief Allocated slots, i.e. `capacity()`. */
    size_type n_alloc = 0;
    /** @brief Constructed elements, i.e. `size()`; invariant `n_stored <= n_alloc`. */
    size_type n_stored = 0;
  };
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_VECTOR_HPP
