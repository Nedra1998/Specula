#ifndef SPECULA_UTIL_PSTD_SPAN_HPP
#define SPECULA_UTIL_PSTD_SPAN_HPP

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <vector>

#include "specula/macros.hpp"

namespace specula::pstd {
  namespace internal {
    template <typename C>
    SPECULA_CPU_GPU constexpr auto get_data_impl(C &c, char /*unused*/) noexcept
        -> decltype(c.data()) {
      return c.data();
    }

    template <typename C>
    SPECULA_CPU_GPU constexpr auto get_data(C &c) noexcept -> decltype(get_data_impl(c, 0)) {
      return get_data_impl(c, 0);
    }

    template <typename C>
    using HasSize = std::is_integral<typename std::decay_t<decltype(std::declval<C &>().size())>>;
    template <typename T, typename C>
    using HasData =
        std::is_convertible<typename std::decay_t<decltype(get_data(std::declval<C &>()))> *,
                            T *const *>;
  } // namespace internal

  inline constexpr std::size_t dynamic_extent = -1;

  /**
   * @class span
   * @brief A reference to a contiguous sequence of objects
   *
   * @tparam T The element type
   *
   * @see https://en.cppreference.com/cpp/container/span
   */
  template <typename T> class span {
  public:
    template <typename C>
    using EnableIfConvertibleFrom =
        typename std::enable_if_t<internal::HasData<T, C>::value && internal::HasSize<C>::value>;

    template <typename U>
    using EnableIfConstView = typename std::enable_if_t<std::is_const_v<T>, U>;

    template <typename U>
    using EnableIfMutableView = typename std::enable_if_t<!std::is_const_v<T>, U>;

    using element_type = T;
    using value_type = typename std::remove_cv_t<T>;
    using size_type = std::size_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;

    SPECULA_CPU_GPU span() : data_(nullptr), size_(0) {}
    SPECULA_CPU_GPU span(T *ptr, size_type n) : data_(ptr), size_(n) {}
    template <std::size_t N> SPECULA_CPU_GPU span(T (&a)[N]) : span(a, N) {}
    SPECULA_CPU_GPU span(std::initializer_list<value_type> v) : span(v.begin(), v.size()) {}

    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfMutableView<V>>
    SPECULA_CPU_GPU explicit span(V &v) noexcept : span(v.data(), v.size()) {}

    template <typename V> span(std::vector<V> &v) noexcept : span(v.data(), v.size()) {}
    template <typename V> span(const std::vector<V> &v) noexcept : span(v.data(), v.size()) {}

    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfConstView<V>>
    SPECULA_CPU_GPU constexpr span(const V &v) noexcept : span(v.data(), v.size()) {}

    SPECULA_CPU_GPU iterator begin() { return data_; }
    SPECULA_CPU_GPU const_iterator begin() const { return data_; }
    SPECULA_CPU_GPU const_iterator cbegin() const { return data_; }
    SPECULA_CPU_GPU iterator end() { return data_ + size_; }
    SPECULA_CPU_GPU const_iterator end() const { return data_ + size_; }
    SPECULA_CPU_GPU const_iterator cend() const { return data_ + size_; }

    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return size_; }
    SPECULA_CPU_GPU [[nodiscard]] bool empty() const { return size_ == 0; }

    SPECULA_CPU_GPU reference operator[](size_type index) {
      // TODO: Uncomment once check.hpp has been fully implemented
      // DASSERT_LT(index, size_);
      return data_[index];
    }

    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      // TODO: Uncomment once check.hpp has been fully implemented
      // DASSERT_LT(index, size_);
      return data_[index];
    }

    SPECULA_CPU_GPU element_type front() const { return data_[0]; }
    SPECULA_CPU_GPU element_type back() const { return data_[size_ - 1]; }

    SPECULA_CPU_GPU pointer data() { return data_; }
    SPECULA_CPU_GPU const_pointer data() const { return data_; }

    SPECULA_CPU_GPU void remove_prefix(size_type count) {
      data_ += count;
      size_ -= count;
    }
    SPECULA_CPU_GPU void remove_suffix(size_type count) { size_ -= count; }

    SPECULA_CPU_GPU span subspan(size_type pos, size_type count = dynamic_extent) {
      size_type np = count < (size() - pos) ? count : (size() - pos);
      return span(data_ + pos, np);
    }

  private:
    pointer data_;
    size_type size_;
  };

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<T> make_span(T *ptr, size_t size) noexcept {
    return span<T>(ptr, size);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU span<T> make_span(T *begin, T *end) noexcept {
    return span<T>(begin, end - begin);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  span<T> make_span(std::vector<T> &v) noexcept {
    return span<T>(v.data(), v.size());
  }

  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU constexpr auto make_span(C &c) noexcept
      -> decltype(make_span(internal::get_data(c), c.size())) {
    return make_span(internal::get_data(c), c.size());
  }

  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU constexpr span<T> make_span(T (&array)[N]) noexcept {
    return span<T>(array, N);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(T *ptr, size_t size) noexcept {
    return span<const T>(ptr, size);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(T *begin, T *end) noexcept {
    return span<const T>(begin, end - begin);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  constexpr span<const T> make_const_span(const std::vector<T> &v) noexcept {
    return span<const T>(v.data(), v.size());
  }

  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU constexpr auto make_const_span(const C &c) noexcept -> decltype(make_span(c)) {
    return make_span(c);
  }

  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(const T (&array)[N]) noexcept {
    return span<const T>(array, N);
  }

} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_SPAN_HPP
