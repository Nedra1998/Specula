#ifndef SPECULA_UTIL_PSTD_VECTOR_HPP
#define SPECULA_UTIL_PSTD_VECTOR_HPP

#include <initializer_list>
#include <iterator>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"
#include "specula/util/pstd/pmr.hpp"
#include "specula/util/pstd/utility.hpp"

namespace specula::pstd {
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

    vector(const Allocator &alloc = {}) : alloc(alloc) {}
    vector(size_type count, const T &value, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(count);
      for (size_type i = 0; i < count; ++i) {
        this->alloc.template construct<value_type>(ptr + i, value);
      }
      n_stored = count;
    }
    vector(size_type count, const Allocator &alloc = {}) : vector(count, T{}, alloc) {}
    vector(const vector &other, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(other.size());
      for (size_type i = 0; i < other.size(); ++i) {
        this->alloc.template construct<value_type>(ptr + i, other[i]);
      }
      n_stored = other.size();
    }
    template <class InputIt>
    vector(InputIt first, InputIt last, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(last - first);
      size_type i = 0;
      for (InputIt iter = first; iter != last; ++iter, ++i) {
        this->alloc.template construct<value_type>(ptr + i, *iter);
      }
      n_stored = n_alloc;
    }
    vector(vector &&other) noexcept
        : alloc(other.alloc), ptr(other.ptr), n_alloc(other.n_alloc), n_stored(other.n_stored) {
      other.n_stored = other.n_alloc = 0;
      other.ptr = nullptr;
    }
    vector(vector &&other, const Allocator &alloc) : alloc(alloc) {
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
    vector(std::initializer_list<value_type> init, const Allocator &alloc = {})
        : vector(init.begin(), init.end(), alloc) {}

    ~vector() {
      clear();
      alloc.deallocate_object(ptr, n_alloc);
    }

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
    vector &operator=(std::initializer_list<value_type> &init) {
      reserve(init.size());
      clear();
      iterator iter = begin();
      for (const auto &value : init) {
        *iter = value;
        ++iter;
      }
      return *this;
    }

    void assign(size_type count, const value_type &value) {
      clear();
      reserve(count);
      for (size_type i = 0; i < count; ++i) {
        push_back(value);
      }
    }

    SPECULA_CPU_GPU iterator begin() { return ptr; }
    SPECULA_CPU_GPU iterator end() { return ptr + n_stored; }
    SPECULA_CPU_GPU const_iterator begin() const { return ptr; }
    SPECULA_CPU_GPU const_iterator end() const { return ptr + n_stored; }
    SPECULA_CPU_GPU const_iterator cbegin() const { return ptr; }
    SPECULA_CPU_GPU const_iterator cend() const { return ptr + n_stored; }

    SPECULA_CPU_GPU reverse_iterator rbegin() { return reverse_iterator(end()); }
    SPECULA_CPU_GPU reverse_iterator rend() { return reverse_iterator(begin()); }
    SPECULA_CPU_GPU const_reverse_iterator rbegin() const { return reverse_iterator(end()); }
    SPECULA_CPU_GPU const_reverse_iterator rend() const { return reverse_iterator(begin()); }

    allocator_type get_allocator() const { return alloc; }
    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return n_stored; }
    SPECULA_CPU_GPU [[nodiscard]] bool empty() const { return n_stored == 0; }
    SPECULA_CPU_GPU [[nodiscard]] size_type max_size() const { return (size_type)-1; }
    SPECULA_CPU_GPU [[nodiscard]] size_type capacity() const { return n_alloc; }

    SPECULA_CPU_GPU reference operator[](size_type index) {
      DASSERT_LT(index, size());
      return ptr[index];
    }
    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      DASSERT_LT(index, size());
      return ptr[index];
    }

    SPECULA_CPU_GPU reference front() { return ptr[0]; }
    SPECULA_CPU_GPU const_reference front() const { return ptr[0]; }
    SPECULA_CPU_GPU reference back() { return ptr[n_stored - 1]; }
    SPECULA_CPU_GPU const_reference back() const { return ptr[n_stored - 1]; }
    SPECULA_CPU_GPU pointer data() { return ptr; }
    SPECULA_CPU_GPU const_pointer data() const { return ptr; }

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

    void clear() {
      for (size_type i = 0; i < n_stored; ++i) {
        alloc.destroy(&ptr[i]);
      }
      n_stored = 0;
    }

    template <class InputIt> iterator insert(const_iterator pos, InputIt first, InputIt last) {
      if (pos == end()) {
        size_t first_offset = size();
        for (auto iter = first; iter != last; ++iter) {
          push_back(iter);
        }
        return begin() + first_offset;
      }
      LOG_CRITICAL("Unimplemented");
    }

    template <class... Args> void emplace_back(Args &&...args) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, std::forward<Args>(args)...);
      ++n_stored;
    }

    void push_back(const T &value) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, value);
      ++n_stored;
    }

    void push_back(T &&value) {
      if (n_alloc == n_stored) {
        reserve(n_alloc == 0 ? 4 : 2 * n_alloc);
      }
      alloc.construct(ptr + n_stored, std::move(value));
      ++n_stored;
    }

    void pop_back() {
      DASSERT(!empty());
      alloc.destroy(ptr + n_stored - 1);
      --n_stored;
    }

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
    Allocator alloc;
    value_type *ptr = nullptr;
    size_type n_alloc = 0, n_stored = 0;
  };
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_VECTOR_HPP
