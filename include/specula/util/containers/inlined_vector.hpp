#ifndef INCLUDE_CONTAINERS_INLINED_VECTOR_HPP_
#define INCLUDE_CONTAINERS_INLINED_VECTOR_HPP_

#include <initializer_list>
#include <iterator>

#include "specula.hpp"
#include "specula/util/pstd.hpp"
#include "util/log.hpp"

namespace specula {
  template <typename T, int N, class Allocator = pstd::pmr::polymorphic_allocator<T>>
  class InlinedVector {
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

    InlinedVector(const Allocator &alloc = {}) : alloc(alloc) {}
    InlinedVector(size_t count, const T *value, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(count);
      for (size_t i = 0; i < count; ++i) {
        this->alloc.template construct<T>(begin() + i, value);
      }
      n_stored = count;
    }
    InlinedVector(size_t count, const Allocator &alloc = {}) : InlinedVector(count, T{}, alloc) {}

    InlinedVector(const InlinedVector &other, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(other.size());
      for (size_t i = 0; i < other.size(); ++i) {
        this->alloc.template construct<T>(begin() + i, other[i]);
      }
      n_stored = other.size();
    }

    template <class InputIt>
    InlinedVector(InputIt first, InputIt last, const Allocator &alloc = {}) : alloc(alloc) {
      reserve(last - first);
      for (InputIt iter = first; iter != last; ++iter, ++n_stored) {
        this->alloc.template construct<T>(begin() + n_stored, *iter);
      }
    }

    InlinedVector(InlinedVector &&other) : alloc(other.alloc) {
      n_stored = other.n_stored;
      n_alloc = other.n_alloc;
      ptr = other.ptr;
      if (other.n_stored <= N) {
        for (int i = 0; i < other.n_stored; ++i) {
          alloc.template construct<T>(fixed + i, std::move(other.fixed[i]));
        }
      } else {
        other.n_stored = 0;
      }
      other.n_alloc = 0;
      other.ptr = nullptr;
    }

    InlinedVector(InlinedVector &&other, const Allocator &alloc) {

      if (alloc == other.alloc) {
        ptr = other.ptr;
        n_alloc = other.n_alloc;
        n_stored = other.n_stored;
        if (other.n_stored <= N) {
          for (int i = 0; i < other.n_stored; ++i) {
            fixed[i] = std::move(other.fixed[i]);
          }
        }
        other.ptr = nullptr;
        other.n_alloc = other.n_stored = 0;
      } else {
        reserve(other.size());
        for (size_t i = 0; i < other.size(); ++i) {
          alloc.template construct<T>(begin() + i, std::move(other[i]));
        }
        n_stored = other.size();
      }
    }

    InlinedVector(std::initializer_list<T> init, const Allocator &alloc = {})
        : InlinedVector(init.begin(), init.end(), alloc) {}

    ~InlinedVector() {
      clear();
      alloc.dealloc_object(ptr, n_alloc);
    }

    void reserve(size_t n) {
      if (capacity() >= n) {
        return;
      }

      T *ra = alloc.template allocate_object<T>(n);
      for (int i = 0; i < n_stored; ++i) {
        alloc.template construct<T>(ra + i, std::move(begin()[i]));
        alloc.destroy(begin() + i);
      }

      alloc.deallocate_object(ptr, n_alloc);
      n_alloc = n;
      ptr = ra;
    }

    void clear() {
      for (int i = 0; i < n_stored; ++i) {
        alloc.destroy(begin() + i);
      }
      n_stored = 0;
    }

    void assign(size_type count, const T &value) {
      clear();
      reserve(count);
      for (size_t i = 0; i < count; ++i) {
        alloc.template construct<T>(begin() + i, value);
      }
      n_stored = count;
    }

    template <class InputIt> void assign(InputIt first, InputIt last) {
      clear();
      reserve(last - first);
      for (InputIt iter = first; iter != last; ++iter, ++n_stored) {
        this->alloc.template construct<T>(begin() + n_stored, *iter);
      }
    }
    void assign(std::initializer_list<T> &init) { assign(init.begin(), init.end()); }

    iterator insert(const_iterator pos, const T &value) { LOG_CRITICAL("TODO"); }
    iterator insert(const_iterator pos, T &&value) { LOG_CRITICAL("TODO"); }
    iterator insert(const_iterator pos, size_t count, const T &value) { LOG_CRITICAL("TODO"); }

    template <typename InputIt> iterator insert(const_iterator pos, InputIt first, InputIt last) {
      if (pos == end()) {
        reserve(size() + (last - first));
        iterator pos = end();
        for (auto iter = first; iter != last; ++iter, ++pos) {
          alloc.template construct<T>(pos * iter);
        }
        n_stored += (last - first);
        return pos;
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
    template <class... Args> void emplace_back(Args &&...args) { LOG_CRITICAL("TODO"); }

    iterator erase(const_iterator cpos) {
      iterator pos = begin() + (cpos - begin());
      while (pos != end() - 1) {
        *pos = std::move(*(pos + 1));
        ++pos;
      }
      alloc.destroy(pos);
      --n_stored;
      return begin() + (cpos - begin());
    }
    iterator erase(const_iterator first, const_iterator last) { LOG_CRITICAL("TODO"); }

    void push_back(const T &value) {
      if (size() == capacity())
        reserve(2 * capacity());
      alloc.template construct<T>(begin() + n_stored, value);
      ++n_stored;
    }

    void push_back(T &&value) {
      if (size() == capacity())
        reserve(2 * capacity());
      alloc.template construct<T>(begin() + n_stored, std::move(value));
      ++n_stored;
    }

    void pop_back() {
      DASSERT(!empty());
      alloc.destroy(begin() + n_stored - 1);
      --n_stored;
    }

    void resize(size_type n) {
      if (n < size()) {
        for (size_t i = n; i < size(); ++i)
          alloc.destroy(begin() + i);
      } else if (n > size()) {
        reserve(n);
        for (size_t i = n_stored; i < n; ++i)
          alloc.construct(begin() + i);
      }
      n_stored = n;
    }

    void resize(size_type count, const value_type &value) { LOG_CRITICAL("TODO"); }

    void swap(InlinedVector &other) { LOG_CRITICAL("TODO"); }

    allocator_type get_allocator() const { return alloc; }

    SPECULA_CPU_GPU size_type size() const { return n_stored; }
    SPECULA_CPU_GPU size_type max_size() const { return (size_t)-1; }
    SPECULA_CPU_GPU size_type capacity() const { return ptr ? n_alloc : N; }

    SPECULA_CPU_GPU bool empty() const { return size() == 0; }

    SPECULA_CPU_GPU iterator begin() { return ptr ? ptr : fixed; }
    SPECULA_CPU_GPU iterator end() { return begin() + n_stored; }
    SPECULA_CPU_GPU const_iterator begin() const { return ptr ? ptr : fixed; }
    SPECULA_CPU_GPU const_iterator end() const { return begin() + n_stored; }
    SPECULA_CPU_GPU const_iterator cbegin() const { return ptr ? ptr : fixed; }
    SPECULA_CPU_GPU const_iterator cend() const { return begin() + n_stored; }

    SPECULA_CPU_GPU reverse_iterator rbegin() { return ptr ? ptr : fixed; }
    SPECULA_CPU_GPU reverse_iterator rend() { return begin() + n_stored; }
    SPECULA_CPU_GPU const_reverse_iterator rbegin() const { return ptr ? ptr : fixed; }
    SPECULA_CPU_GPU const_reverse_iterator rend() const { return begin() + n_stored; }
    SPECULA_CPU_GPU const_reverse_iterator rcbegin() const { return ptr ? ptr : fixed; }
    SPECULA_CPU_GPU const_reverse_iterator rcend() const { return begin() + n_stored; }

    SPECULA_CPU_GPU reference front() { return *begin(); }
    SPECULA_CPU_GPU const_reference front() const { return *begin(); }
    SPECULA_CPU_GPU reference back() { return *(begin() + n_stored - 1); }
    SPECULA_CPU_GPU const_reference back() const { return *(begin() + n_stored - 1); }
    SPECULA_CPU_GPU pointer data() { return ptr ? ptr : fixed; }
    SPECULA_CPU_GPU const_pointer data() const { return ptr ? ptr : fixed; }

    SPECULA_CPU_GPU reference operator[](size_type index) {
      DASSERT_LT(index, size());
      return begin()[index];
    }

    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      DASSERT_LT(index, size());
      return begin()[index];
    }

    InlinedVector &operator=(const InlinedVector &other) {
      if (this == &other) {
        return *this;
      }

      clear();
      reserve(other.size());
      for (size_t i = 0; i < other.size(); ++i) {
        alloc.template construct<T>(begin() + i, other[i]);
      }
      n_stored = other.size();
      return *this;
    }

    InlinedVector &operator=(InlinedVector &&other) {
      if (this == &other) {
        return *this;
      }

      clear();
      if (alloc == other.alloc) {
        pstd::swap(ptr, other.ptr);
        pstd::swap(n_alloc, other.n_alloc);
        pstd::swap(n_stored, other.n_stored);
        if (n_stored > 0 && !ptr) {
          for (int i = 0; i < n_stored; ++i) {
            alloc.template construct<T>(fixed + i, std::move(other.fixed[i]));
          }
          other.n_stored = n_stored;
        }
      } else {
        reserve(other.size());
        for (size_t i = 0; i < other.size(); ++i) {
          alloc.template construct<T>(begin() + i, std::move(other[i]));
        }
        n_stored = other.size();
      }
      return *this;
    }

    InlinedVector &operator=(std::initializer_list<T> &init) {
      clear();
      reserve(init.size());
      for (const auto &value : init) {
        alloc.template construct<T>(begin() + n_stored, value);
        ++n_stored;
      }
      return *this;
    }

  private:
    Allocator alloc;
    T *ptr = nullptr;
    union {
      T fixed[N];
    };
    size_t n_alloc = 0, n_stored = 0;
  };
} // namespace specula

#endif // INCLUDE_CONTAINERS_INLINED_VECTOR_HPP_
