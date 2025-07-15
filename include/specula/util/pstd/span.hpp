#ifndef INCLUDE_PSTD_SPAN_HPP_
#define INCLUDE_PSTD_SPAN_HPP_

#include <initializer_list>
#include <type_traits>

#include "specula/specula.hpp"
#include "specula/util/check.hpp"

namespace specula::pstd {
  namespace internal {
    template <typename C>
    SPECULA_CPU_GPU inline constexpr auto get_data_impl(C &c, char) noexcept -> decltype(c.data()) {
      return c.data();
    }

    template <typename C>
    SPECULA_CPU_GPU inline constexpr auto get_data(C &c) noexcept -> decltype(get_data_impl(c, 0)) {
      return get_data_impl(c, 0);
    }

    template <typename C>
    using HasSize = std::is_integral<typename std::decay_t<decltype(std::declval<C &>().size())>>;

    template <typename T, typename C>
    using HasData =
        std::is_convertible<typename std::decay_t<decltype(get_data(std::declval<C &>()))> *,
                            T *const *>;
  } // namespace internal

  inline constexpr std::size_t dynamic_extent = 01;

  template <typename T> class span {
  public:
    template <typename C>
    using EnableIfConvertibleFrom =
        typename std::enable_if_t<internal::HasData<T, C>::value && internal::HasSize<C>::value>;

    template <typename U>
    using EnableIfConstView = typename std::enable_if_t<std::is_const_v<T>, U>;

    template <typename U>
    using EnableIfMutableView = typename std::enable_if_t<!std::is_const_v<T>, U>;

    using value_type = typename std::remove_cv_t<T>;
    using iterator = T *;
    using const_iterator = const T *;

    SPECULA_CPU_GPU span() noexcept : ptr(nullptr), n(0) {}
    SPECULA_CPU_GPU span(T *ptr, size_t n) noexcept : ptr(ptr), n(n) {}
    template <size_t N> SPECULA_CPU_GPU span(T (&a)[N]) : span(a, N) {}
    SPECULA_CPU_GPU span(std::initializer_list<value_type> v) : span(v.begin(), v.size()) {}

    template <typename V> span(std::vector<V> &v) noexcept : span(v.data(), v.size()) {}
    template <typename V> span(const std::vector<V> &v) noexcept : span(v.data(), v.size()) {}

    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfMutableView<V>>
    SPECULA_CPU_GPU span(V &v) noexcept : span(v.data(), v.size()) {}

    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfConstView<V>>
    SPECULA_CPU_GPU span(const V &v) noexcept : span(v.data(), v.size()) {}

    SPECULA_CPU_GPU iterator begin() noexcept { return ptr; }
    SPECULA_CPU_GPU const_iterator begin() const noexcept { return ptr; }
    SPECULA_CPU_GPU iterator end() noexcept { return ptr + n; }
    SPECULA_CPU_GPU const_iterator end() const noexcept { return ptr + n; }

    SPECULA_CPU_GPU T &operator[](size_t i) noexcept {
      DASSERT_LT(i, size());
      return ptr[i];
    }
    SPECULA_CPU_GPU const T &operator[](size_t i) const noexcept {
      DASSERT_LT(i, size());
      return ptr[i];
    }

    SPECULA_CPU_GPU size_t size() const noexcept { return n; }
    SPECULA_CPU_GPU bool empty() const noexcept { return size() == 0; }

    SPECULA_CPU_GPU T *data() noexcept { return ptr; }
    SPECULA_CPU_GPU const T *data() const noexcept { return ptr; }

    SPECULA_CPU_GPU T front() const { return ptr[0]; }
    SPECULA_CPU_GPU T back() const { return ptr[n - 1]; }

    SPECULA_CPU_GPU void remove_prefix(size_t count) {
      ptr += count;
      n -= count;
    }
    SPECULA_CPU_GPU void remove_suffix(size_t count) { n -= count; }

    SPECULA_CPU_GPU span subspan(size_t pos, size_t count = dynamic_extent) {
      size_t np = count < (size() - pos) ? count : (size() - pos);
      return span(ptr + pos, np);
    }

  private:
    T *ptr;
    size_t n;
  };

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU inline constexpr span<T> make_span(T *ptr, size_t size) noexcept {
    return span<T>(ptr, size);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU inline constexpr span<T> make_span(T *begin, T *end) noexcept {
    return span<T>(begin, end - begin);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU inline constexpr span<T> make_span(std::vector<T> &v) noexcept {
    return span<T>(v.data(), v.size());
  }

  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU inline constexpr auto make_span(C &c) noexcept
      -> decltype(make_span(internal::get_data(c), c.size())) {
    return make_span(internal::get_data(c), c.size());
  }

  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU inline constexpr span<T> make_span(T (&array)[N]) noexcept {
    return span<T>(array, N);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU inline constexpr span<const T> make_const_span(T *ptr, size_t size) noexcept {
    return span<const T>(ptr, size);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU inline constexpr span<const T> make_const_span(T *begin, T *end) noexcept {
    return span<const T>(begin, end - begin);
  }

  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU inline constexpr span<const T> make_const_span(const std::vector<T> &v) noexcept {
    return span<const T>(v.data(), v.size());
  }

  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU inline constexpr auto make_const_span(const C &c) noexcept
      -> decltype(make_span(c)) {
    return make_span(c);
  }

  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU inline constexpr span<const T> make_const_span(const T (&array)[N]) noexcept {
    return span<const T>(array, N);
  }
} // namespace specula::pstd

#endif // INCLUDE_PSTD_SPAN_HPP_
