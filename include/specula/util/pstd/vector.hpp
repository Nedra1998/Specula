/**
 * @file
 * @brief A dynamic array container usable on both the host (CPU) and device (GPU)
 *
 * This header defines `specula::pstd::vector`, a standard-library-like dynamic array whose storage
 * is allocated through a `pmr::polymorphic_allocator` (or a user-supplied allocator). It is a
 * drop-in replacement for `std::vector` for the subset of functionality needed by the renderer,
 * with the additional property that every public member is annotated `SPECULA_CPU_GPU` so that it
 * can be called from device code.
 *
 * The container owns its buffer and destroys every element it owns when it is destructed or
 * cleared. Elements are stored in contiguous memory and iterators are raw pointers, so the usual
 * pointer-arithmetic idioms of the C++ standard library apply.
 */

#ifndef SPECULA_UTIL_PSTD_VECTOR_HPP
#define SPECULA_UTIL_PSTD_VECTOR_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <initializer_list>
#include <iterator>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/pstd/pmr.hpp"
#include "specula/util/pstd/utility.hpp"

namespace specula::pstd {
  /**
   * @brief A dynamic array container usable on both the host (CPU) and device (GPU)
   *
   * `vector` is a dynamic array whose elements are allocated, destroyed and constructed through an
   * allocator of type `Allocator`, which defaults to `pmr::polymorphic_allocator<T>`. The container
   * keeps two separate counts: `n_alloc`, the number of elements for which storage has been
   * allocated, and `n_stored`, the number of elements that have been constructed and are therefore
   * owned by this object. All of the public accessors and mutators are expressed in terms of these
   * two quantities, which mirrors the semantics of `std::vector`.
   *
   * The buffer is always contiguous and is exposed through raw pointers, so `vector` can be used in
   * device code and with the usual algorithms that take raw iterators. Note that, unlike
   * `std::vector`, `insert` is not implemented and logs a critical error if invoked.
   *
   * @tparam T The element type of the container.
   * @tparam Allocator The allocator used for the internal buffer.
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

    /**
     * @brief Constructs an empty vector.
     *
     * The resulting vector owns no elements and has a null data pointer.
     *
     * @param alloc The allocator to use for the buffer. The default is the allocator deduced from
     * `T`.
     */
    vector(const Allocator &alloc = {}) : alloc(alloc) {}

    /**
     * @brief Constructs a vector with a given number of repeated elements.
     *
     * Allocates space for `count` elements and value-initialises each one from `value`. The
     * elements are constructed through `alloc`.
     *
     * @param count The number of elements to construct.
     * @param value The value copied into every element.
     * @param alloc The allocator to use for the buffer.
     */
    vector(size_type count, const T &value, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(count);
      for (size_type i = 0; i < count; ++i) {
        this->alloc.template construct<value_type>(ptr + i, value);
      }
      n_stored = count;
    }

    /**
     * @brief Constructs a vector with a given number of default-inserted elements.
     *
     * Delegates to the two-argument constructor above with a default-constructed value for each
     * element.
     *
     * @param count The number of elements to construct.
     * @param alloc The allocator to use for the buffer.
     */
    vector(size_type count, const Allocator &alloc = {}) : vector(count, T{}, alloc) {}

    /**
     * @brief Copy constructor.
     *
     * Performs a deep copy: the new vector allocates its own buffer and default-constructs and
     * copy-constructs each element of `other`. If `alloc` is supplied it is used for the new
     * buffer, otherwise `other`'s allocator is copied.
     *
     * @param other The vector to copy.
     * @param alloc The allocator to use for the new buffer.
     */
    vector(const vector &other, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(other.size());
      for (size_type i = 0; i < other.size(); ++i) {
        this->alloc.template construct<value_type>(ptr + i, other[i]);
      }
      n_stored = other.size();
    }

    /**
     * @brief Constructs a vector from a pair of input iterators.
     *
     * Copies the range `[first, last)` into a freshly allocated buffer.
     *
     * @tparam InputIt An input iterator.
     * @param first The first element of the range.
     * @param last One past the last element of the range.
     * @param alloc The allocator to use for the new buffer.
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

    /**
     * @brief Move constructor.
     *
     * Transfers the ownership of the buffer from `other`, leaving the source in a valid but empty
     * state with a null data pointer.
     *
     * @param other The vector to steal the buffer from.
     */
    vector(vector &&other) noexcept
        : alloc(other.alloc), ptr(other.ptr), n_alloc(other.n_alloc), n_stored(other.n_stored) {
      other.n_stored = other.n_alloc = 0;
      other.ptr = nullptr;
    }

    /**
     * @brief Move constructor with an explicit allocator.
     *
     * If `alloc` and `other`'s allocator are equivalent, the buffer is moved over without copying.
     * Otherwise the elements are copied into a newly allocated buffer using `alloc`.
     *
     * @param other The vector to steal from or copy.
     * @param alloc The allocator to use for the new buffer (when it differs from `other`'s
     * allocator).
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

    /**
     * @brief Constructs a vector from an initializer list.
     *
     * Delegates to the iterator-pair constructor by wrapping the initializer list's begin and end
     * iterators.
     *
     * @param init The initializer list.
     * @param alloc The allocator to use for the buffer.
     */
    vector(std::initializer_list<value_type> init, const Allocator &alloc = {})
        : vector(init.begin(), init.end(), alloc) {}

    /**
     * @brief Destructor.
     *
     * Destroys every stored element and deallocates the buffer through `alloc`. If the vector is
     * empty, `ptr` is null and nothing is freed.
     */
    ~vector() {
      clear();
      alloc.deallocate_object(ptr, n_alloc);
    }

    /**
     * @brief Copy assignment operator.
     *
     * Clears the current contents and then deep-copies the elements of `other`. Self-assignment is
     * detected and is a no-op. If the two vectors use different allocators, the new elements are
     * copy-constructed through the current vector's allocator.
     *
     * @param other The vector to copy from.
     * @return A reference to this vector.
     */
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
     * @brief Move assignment operator.
     *
     * If the two vectors use equivalent allocators, the buffers are swapped. Otherwise the current
     * contents are destroyed and the elements of `other` are move-constructed through the current
     * allocator.
     *
     * @param other The vector to steal from.
     * @return A reference to this vector.
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
     * @brief Assigns the contents from an initializer list.
     *
     * Reserves space for the list, clears the current contents and assigns each value in order.
     *
     * @param init The initializer list to assign.
     * @return A reference to this vector.
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

    /**
     * @brief Assigns a sequence of `count` copies of `value`.
     *
     * Clears the current contents and then appends `count` copies of `value`.
     *
     * @param count The number of elements to assign.
     * @param value The value copied into every element.
     */
    void assign(size_type count, const value_type &value) {
      clear();
      reserve(count);
      for (size_type i = 0; i < count; ++i) {
        push_back(value);
      }
    }

    /**
     * @brief Returns an iterator to the first element.
     *
     * This is equivalent to `data()`. The resulting iterator is only valid when the vector is
     * non-empty.
     *
     * @return An iterator to the first element.
     */
    SPECULA_CPU_GPU iterator begin() { return ptr; }

    /**
     * @brief Returns an iterator one past the last element.
     *
     * This is equivalent to `data() + size()`.
     *
     * @return An iterator one past the last element.
     */
    SPECULA_CPU_GPU iterator end() { return ptr + n_stored; }

    /**
     * @brief Returns a const iterator to the first element.
     *
     * @return A const iterator to the first element.
     */
    SPECULA_CPU_GPU const_iterator begin() const { return ptr; }

    /**
     * @brief Returns a const iterator one past the last element.
     *
     * @return A const iterator one past the last element.
     */
    SPECULA_CPU_GPU const_iterator end() const { return ptr + n_stored; }

    /**
     * @brief Returns a const iterator to the first element.
     *
     * @return A const iterator to the first element.
     */
    SPECULA_CPU_GPU const_iterator cbegin() const { return ptr; }

    /**
     * @brief Returns a const iterator one past the last element.
     *
     * @return A const iterator one past the last element.
     */
    SPECULA_CPU_GPU const_iterator cend() const { return ptr + n_stored; }

    /**
     * @brief Returns a reverse iterator to the last element.
     *
     * @return A reverse iterator to the last element.
     */
    SPECULA_CPU_GPU reverse_iterator rbegin() { return reverse_iterator(end()); }

    /**
     * @brief Returns a reverse iterator before the first element.
     *
     * @return A reverse iterator before the first element.
     */
    SPECULA_CPU_GPU reverse_iterator rend() { return reverse_iterator(begin()); }

    /**
     * @brief Returns a const reverse iterator to the last element.
     *
     * @return A const reverse iterator to the last element.
     */
    SPECULA_CPU_GPU const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }

    /**
     * @brief Returns a const reverse iterator before the first element.
     *
     * @return A const reverse iterator before the first element.
     */
    SPECULA_CPU_GPU const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    /**
     * @brief Returns the allocator used by this vector.
     *
     * @return The allocator used by this vector.
     */
    allocator_type get_allocator() const { return alloc; }

    /**
     * @brief Returns the number of stored (constructed) elements.
     *
     * @return The number of stored elements.
     */
    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return n_stored; }

    /**
     * @brief Returns whether or not the vector is empty.
     *
     * @return `true` if the vector holds no elements.
     */
    SPECULA_CPU_GPU [[nodiscard]] bool empty() const { return n_stored == 0; }

    /**
     * @brief Returns the maximum number of elements the vector can hold.
     *
     * @return The maximum size, which is `(size_type)-1`.
     */
    SPECULA_CPU_GPU [[nodiscard]] size_type max_size() const { return (size_type)-1; }

    /**
     * @brief Returns the number of allocated elements.
     *
     * This may be greater than `size()`, since the vector may hold more storage than it currently
     * owns. It corresponds to `capacity()` in the standard library.
     *
     * @return The number of allocated elements.
     */
    SPECULA_CPU_GPU [[nodiscard]] size_type capacity() const { return n_alloc; }

    /**
     * @brief Returns a reference to the element at the given index.
     *
     * The index is bounds-checked against `size()` using `DASSERT_LT`.
     *
     * @param index The index of the element.
     * @return A reference to the element at `index`.
     */
    SPECULA_CPU_GPU reference operator[](size_type index) {
      DASSERT_LT(index, size());
      return ptr[index];
    }

    /**
     * @brief Returns a const reference to the element at the given index.
     *
     * The index is bounds-checked against `size()` using `DASSERT_LT`.
     *
     * @param index The index of the element.
     * @return A const reference to the element at `index`.
     */
    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      DASSERT_LT(index, size());
      return ptr[index];
    }

    /**
     * @brief Returns a reference to the first element.
     *
     * @return A reference to the first element.
     */
    SPECULA_CPU_GPU reference front() { return ptr[0]; }

    /**
     * @brief Returns a const reference to the first element.
     *
     * @return A const reference to the first element.
     */
    SPECULA_CPU_GPU const_reference front() const { return ptr[0]; }

    /**
     * @brief Returns a reference to the last element.
     *
     * @return A reference to the last element.
     */
    SPECULA_CPU_GPU reference back() { return ptr[n_stored - 1]; }

    /**
     * @brief Returns a const reference to the last element.
     *
     * @return A const reference to the last element.
     */
    SPECULA_CPU_GPU const_reference back() const { return ptr[n_stored - 1]; }

    /**
     * @brief Returns a pointer to the first element.
     *
     * This is equivalent to `data()`. The pointer may be null when the vector is empty.
     *
     * @return A pointer to the first element.
     */
    SPECULA_CPU_GPU pointer data() { return ptr; }

    /**
     * @brief Returns a const pointer to the first element.
     *
     * @return A const pointer to the first element.
     */
    SPECULA_CPU_GPU const_pointer data() const { return ptr; }

    /**
     * @brief Ensures that the vector can hold at least `n` elements.
     *
     * If the vector already has capacity for `n` elements, this is a no-op. Otherwise a new buffer
     * of `n` elements is allocated through `alloc` and the currently stored elements are moved into
     * it, after which the old buffer is deallocated. The vector's size is unchanged.
     *
     * @param n The minimum number of elements to be able to hold.
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

    /**
     * @brief Destroys every stored element without deallocating the buffer.
     *
     * After this call, `size()` is zero but `capacity()` is unchanged. The buffer may be reused by
     * subsequent growth operations.
     */
    void clear() {
      for (size_type i = 0; i < n_stored; ++i) {
        alloc.destroy(&ptr[i]);
      }
      n_stored = 0;
    }

    /**
     * @brief Inserts a range of elements before `pos`.
     *
     * **This function is not implemented.** When `pos` is at the end of the vector the elements are
     * simply appended via `push_back`; for any other position it logs a critical error. Any use of
     * this overload should be replaced with a reallocation-and-copy-and-erase sequence.
     *
     * @tparam InputIt An input iterator.
     * @param pos An iterator into the vector.
     * @param first The first element of the range to insert.
     * @param last One past the last element of the range to insert.
     * @return An iterator to the first inserted element.
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
    }

    /**
     * @brief Appends an element constructed in-place at the end of the vector.
     *
     * Grows the buffer as needed before constructing the element. If the vector is empty, the
     * initial capacity is 4; otherwise the capacity is doubled on each growth, which gives
     * amortised `O(1)` insertion.
     *
     * @tparam Args The argument types of the element constructor.
     * @param args The arguments forwarded to the element's constructor.
     */
    template <class... Args> void emplace_back(Args &&...args) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, std::forward<Args>(args)...);
      ++n_stored;
    }

    /**
     * @brief Appends a copy of `value` at the end of the vector.
     *
     * Grows the buffer as needed before copying the value. If the vector is empty, the initial
     * capacity is 4; otherwise the capacity is doubled on each growth.
     *
     * @param value The value to append.
     */
    void push_back(const T &value) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, value);
      ++n_stored;
    }

    /**
     * @brief Appends a move of `value` at the end of the vector.
     *
     * Grows the buffer as needed before moving the value. If the vector is empty, the initial
     * capacity is 4; otherwise the capacity is doubled on each growth.
     *
     * @param value The value to append.
     */
    void push_back(T &&value) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, std::move(value));
      ++n_stored;
    }

    /**
     * @brief Removes the last element from the vector.
     *
     * @pre The vector is non-empty.
     */
    void pop_back() {
      DASSERT(!empty());
      alloc.destroy(ptr + n_stored - 1);
      --n_stored;
    }

    /**
     * @brief Resizes the vector to `n` elements.
     *
     * If `n` is smaller than the current size, the excess elements are destroyed. If `n_alloc` is
     * zeroed out as well when `n` is zero, so that the buffer is released. If `n` is larger, the
     * buffer is grown and the new elements are default-constructed.
     *
     * @param n The new size of the vector.
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
     * @brief Swaps the contents of this vector with `other`.
     *
     * If the two vectors use equivalent allocators, the three buffers and counts are simply
     * swapped. Otherwise a temporary is used to copy this vector and the three objects are moved in
     * a cycle, so that no element is lost regardless of the allocators involved.
     *
     * @param other The vector to swap with.
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
    /**
     * @brief The allocator used for the internal buffer.
     *
     * Members are ordered to mirror the layout of the standard library: the allocator first,
     * followed by the data pointer and the two counts.
     */
    Allocator alloc;

    /**
     * @brief A pointer to the allocated buffer.
     *
     * May be `nullptr` when the vector owns no storage, which is the case for an empty vector or
     * after a move-from.
     */
    value_type *ptr = nullptr;

    /**
     * @brief The number of elements for which storage has been allocated.
     *
     * This is the analogue of `capacity()` and may be greater than `n_stored`.
     */
    size_type n_alloc = 0;

    /**
     * @brief The number of elements which are stored in the buffer.
     *
     * This is the analogue of `size()` and must be less than `n_alloc`.
     */
    size_type n_stored = 0;
  };
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_VECTOR_HPP
