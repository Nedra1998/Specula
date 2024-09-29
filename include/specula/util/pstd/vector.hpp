#ifndef INCLUDE_PSTD_VECTOR_HPP_
#define INCLUDE_PSTD_VECTOR_HPP_

#include <cstddef>
#include <initializer_list>
#include <iterator>

#include "specula/specula.hpp"
#include "specula/util/check.hpp"
#include "specula/util/log.hpp"
#include "specula/util/pstd/pmr.hpp"

namespace specula::pstd {
  template <typename T> SPECULA_CPU_GPU inline void swap(T &a, T &b);

  template <typename T, class Allocator = pmr::polymorphic_allocator<T>> class vector {
  public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = T *;
    using const_pointer = const T *;
    using iterator = T *;
    using const_iterator = const T *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    vector(const Allocator &alloc = {}) noexcept : alloc(alloc) {}
    vector(size_t count, const T &value, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(count);
      for (size_t i = 0; i < count; ++i)
        this->alloc.template construct<T>(ptr + i, value);
      n_stored = count;
    }
    vector(size_t count, const Allocator &alloc = {}) : vector(count, T{}, alloc) {}
    vector(const vector &other, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(other.size());
      for (size_t i = 0; i < other.size(); ++i)
        this->alloc.template construct<T>(ptr + i, other[i]);
      n_stored = other.size();
    }
    template <class InputIt>
    vector(InputIt first, InputIt last, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(last - first);
      size_t i = 0;
      for (InputIt iter = first; iter != last; ++iter, ++i)
        this->alloc.template construct<T>(ptr + i, *iter);
      n_stored = n_alloc;
    }
    vector(vector &&other) : alloc(other.alloc) {
      n_stored = other.n_stored;
      n_alloc = other.n_alloc;
      ptr = other.ptr;

      other.n_stored = other.n_alloc = 0;
      other.ptr = nullptr;
    }
    vector(vector &&other, const Allocator &alloc) : alloc(alloc) {
      if (alloc == other.alloc) {
        ptr = other.ptr;
        n_alloc = other.n_alloc;
        n_stored = other.n_stored;

        other.n_alloc = other.n_stored = 0;
        other.ptr = nullptr;
      } else {
        reserve(other.size());
        for (size_t i = 0; i < other.size(); ++i)
          this->alloc.template construct<T>(ptr + i, std::move(other[i]));
        n_stored = other.size();
      }
    }
    vector(std::initializer_list<T> init, const Allocator &alloc = {})
        : vector(init.begin(), init.end(), alloc) {}

    ~vector() {
      clear();
      alloc.deallocate_object(ptr, n_alloc);
    }

    vector &operator=(const vector &other) {
      if (this == &other)
        return *this;

      clear();
      reserve(other.size());
      for (size_t i = 0; i < other.size(); ++i)
        alloc.template construct<T>(ptr + i, other[i]);
      n_stored = other.size();
      return *this;
    }
    vector &operator=(vector &&other) {
      if (this == &other)
        return *this;

      if (alloc == other.alloc) {
        pstd::swap(ptr, other.ptr);
        pstd::swap(n_alloc, other.n_alloc);
        pstd::swap(n_stored, other.n_stored);
      } else {
        clear();
        reserve(other.size());
        for (size_t i = 0; i < other.size(); ++i)
          alloc.template construct<T>(ptr + i, std::move(other[i]));
        n_stored = other.size();
      }

      return *this;
    }

    vector &operator=(std::initializer_list<T> &init) {
      reserve(init.size());
      clear();
      iterator iter = begin();
      for (const auto &value : init) {
        *iter = value;
        ++iter;
      }
      return *this;
    }

    void assign(size_type count, const T &value) {
      clear();
      reserve(count);
      for (size_t i = 0; i < count; ++i)
        push_back(value);
    }

    template <class InputIt> void assign(InputIt first, InputIt last) {
      clear();
      reserve(last - first);
      for (InputIt iter = first; iter != last; ++iter)
        push_back(*iter);
    }
    void assign(std::initializer_list<T> init) { assign(init.begin(), init.end()); }

    void reserve(size_t n) {
      if (n_alloc >= n)
        return;

      T *ra = alloc.template allocate_object<T>(n);
      for (size_t i = 0; i < n_stored; ++i) {
        alloc.template construct<T>(ra + i, std::move(begin()[i]));
        alloc.destroy(begin() + i);
      }
      alloc.deallocate_object(ptr, n_alloc);
      n_alloc = n;
      ptr = ra;
    }

    void clear() {
      for (size_t i = 0; i < n_stored; ++i)
        alloc.destroy(&ptr[i]);
      n_stored = 0;
    }

    allocator_type get_allocator() const { return alloc; }
    SPECULA_CPU_GPU size_t size() const noexcept { return n_stored; }
    SPECULA_CPU_GPU size_t capacity() const noexcept { return n_alloc; }
    SPECULA_CPU_GPU bool empty() const noexcept { return size() == 0; }
    SPECULA_CPU_GPU size_t max_size() const noexcept { return (size_t)-1; }

    SPECULA_CPU_GPU iterator begin() noexcept { return ptr; }
    SPECULA_CPU_GPU const_iterator begin() const noexcept { return ptr; }
    SPECULA_CPU_GPU const_iterator cbegin() const noexcept { return ptr; }
    SPECULA_CPU_GPU iterator end() noexcept { return ptr + n_stored; }
    SPECULA_CPU_GPU const_iterator end() const noexcept { return ptr + n_stored; }
    SPECULA_CPU_GPU const_iterator cend() const noexcept { return ptr + n_stored; }

    SPECULA_CPU_GPU reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    SPECULA_CPU_GPU const_reverse_iterator rbegin() const noexcept {
      return const_reverse_iterator(end());
    }
    SPECULA_CPU_GPU const_reverse_iterator crbegin() const noexcept {
      return const_reverse_iterator(end());
    }
    SPECULA_CPU_GPU reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    SPECULA_CPU_GPU const_reverse_iterator rend() const noexcept {
      return const_reverse_iterator(begin());
    }
    SPECULA_CPU_GPU const_reverse_iterator crend() const noexcept {
      return const_reverse_iterator(begin());
    }

    SPECULA_CPU_GPU reference operator[](size_t i) noexcept {
      DASSERT_LT(i, size());
      return ptr[i];
    }
    SPECULA_CPU_GPU const_reference operator[](size_t i) const noexcept {
      DASSERT_LT(i, size());
      return ptr[i];
    }

    SPECULA_CPU_GPU reference front() noexcept { return ptr[0]; }
    SPECULA_CPU_GPU const_reference front() const noexcept { return ptr[0]; }
    SPECULA_CPU_GPU reference back() noexcept { return ptr[n_stored - 1]; }
    SPECULA_CPU_GPU const_reference back() const noexcept { return ptr[n_stored - 1]; }

    SPECULA_CPU_GPU pointer data() noexcept { return ptr; }
    SPECULA_CPU_GPU const_pointer data() const noexcept { return ptr; }

    iterator insert(const_iterator pos, const T &value) { LOG_CRITICAL("TODO"); }
    iterator insert(const_iterator pos, T &&value) { LOG_CRITICAL("TODO"); }
    iterator insert(const_iterator pos, size_t count, const T &value) { LOG_CRITICAL("TODO"); }

    template <class InputIt> iterator insert(const_iterator pos, InputIt first, InputIt last) {
      if (pos == end()) {
        size_t first_offset = size();
        for (auto iter = first; iter != last; ++iter)
          push_back(*iter);
        return begin() + first_offset;
      } else {
        LOG_CRITICAL("TODO");
      }
    }

    iterator insert(const_iterator pos, std::initializer_list<T> init) {
      return insert(pos, init.begin(), init.end());
    }

    template <class... Args> iterator emplace(const_iterator pos, Args &&...args) {
      LOG_CRITICAL("TODO");
    }

    template <class... Args> void emplace_back(Args &&...args) {
      if (n_stored == n_alloc)
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      alloc.template construct<T>(ptr + n_stored, std::forward<Args>(args)...);
      ++n_stored;
    }

    iterator erase(const_iterator pos) { LOG_CRITICAL("TODO"); }
    iterator erase(const_iterator first, const_iterator last) { LOG_CRITICAL("TODO"); }

    void push_back(const T &value) {
      if (n_stored == n_alloc)
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      alloc.template construct<T>(ptr + n_stored, value);
      ++n_stored;
    }

    void push_back(T &&value) {
      if (n_stored == n_alloc)
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      alloc.template construct<T>(ptr + n_stored, std::move(value));
      ++n_stored;
    }

    void pop_back() {
      DASSERT(n_stored > 0);
      alloc.destroy(ptr + n_stored - 1);
      --n_stored;
    }

    void resize(size_type n) {
      if (n < size()) {
        for (size_t i = n; i < size(); ++i)
          alloc.destroy(ptr + i);
        if (n == 0) {
          alloc.deallocate_object(ptr, n_alloc);
          ptr = nullptr;
          n_alloc = 0;
        }
      } else if (n > size()) {
        reserve(n);
        for (size_t i = size(); i < n; ++i)
          alloc.construct(ptr + i);
      }
      n_stored = n;
    }

    void resize(size_type count, const value_type &value) { LOG_CRITICAL("TODO"); }

    void swap(vector &other) {
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
    Allocator alloc;
    T *ptr = nullptr;
    size_t n_alloc = 0, n_stored = 0;
  };
} // namespace specula::pstd

#endif // INCLUDE_PSTD_VECTOR_HPP_
