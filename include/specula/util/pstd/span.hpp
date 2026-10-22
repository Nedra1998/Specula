/**
 * @file
 * @brief A reference to a contiguous sequence of objects usable on both the host (CPU) and device
 * (GPU)
 *
 * This header defines `specula::pstd::span`, a non-owning view over a contiguous sequence of
 * objects, together with factory functions for constructing spans from raw pointers, arrays,
 * containers and other span-like types. It follows the `std::span` interface closely, with the
 * additional property that every public member is annotated `SPECULA_CPU_GPU` so that it can be
 * called from device code.
 *
 * A span owns no memory and carries no allocator: it is always constructed from a pointer and a
 * length, and it is not safe to store across allocations.
 */

#ifndef SPECULA_UTIL_PSTD_SPAN_HPP
#define SPECULA_UTIL_PSTD_SPAN_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <vector>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"

namespace specula::pstd {
  namespace internal {
    /**
     * @brief Attempts to extract the data pointer of a container-like type.
     *
     * SFINAE-detects whether `C` exposes a `data()` member and returns it if so. This lets
     * `make_span` work uniformly for raw pointers, C arrays, `std::vector` and any other type
     * exposing `data()` and `size()`.
     *
     * @tparam C The type of the container.
     * @param c A reference to the container.
     * @param unused A discarded character parameter used to select the SFINAE overload.
     * @return The data pointer of `c`, or `void` if `C` has no `data()` member.
     */
    template <typename C>
    SPECULA_CPU_GPU constexpr auto get_data_impl(C &c, char /*unused*/) noexcept
        -> decltype(c.data()) {
      return c.data();
    }

    /**
     * @brief Returns the data pointer of a container-like type.
     *
     * The SFINAE-friendly entry point used by `make_span`. Dispatches to `get_data_impl` with a
     * dummy `char` argument so that the return type is computed in an unevaluated context.
     *
     * @tparam C The type of the container.
     * @param c A reference to the container.
     * @return The data pointer of `c`.
     */
    template <typename C>
    SPECULA_CPU_GPU constexpr auto get_data(C &c) noexcept -> decltype(get_data_impl(c, 0)) {
      return get_data_impl(c, 0);
    }

    /**
     * @brief Trait that detects whether `C` exposes an integral `size()` member.
     *
     * @tparam C The type of the container.
     * @return `true` if `C::size()` is integral.
     */
    template <typename C>
    using HasSize = std::is_integral<typename std::decay_t<decltype(std::declval<C &>().size())>>;

    /**
     * @brief Trait that detects whether `C` exposes data that can be read as a `T*`.
     *
     * `HasData<T, C>` is `true` when the data pointer returned by `C::data()` is convertible to `T
     * *const *`, i.e. when the container's element type is convertible to `T`. This is what allows
     * `make_span` to convert between span types.
     *
     * @tparam T The desired element type.
     * @tparam C The type of the container.
     * @return `true` if `C`'s data pointer is convertible to `T *const *`.
     */
    template <typename T, typename C>
    using HasData =
        std::is_convertible<typename std::decay_t<decltype(get_data(std::declval<C &>()))> *,
                            T *const *>;
  } // namespace internal

  /**
   * @brief The sentinel value used to indicate that a span has a dynamic extent.
   *
   * When passed to `subspan`, a count of `dynamic_extent` means that the remainder of the span is
   * returned.
   */
  inline constexpr std::size_t dynamic_extent = -1;

  /**
   * @brief A reference to a contiguous sequence of objects.
   *
   * `span` is a non-owning view over a contiguous sequence of `T` objects. It stores a data pointer
   * and a length, and never allocates, deallocates or destroys the memory it references. It is safe
   * to use on the GPU, but not safe to store across an allocation that may reallocate the
   * underlying buffer.
   *
   * @tparam T The element type of the span.
   *
   * @see https://en.cppreference.com/cpp/container/span
   */
  template <typename T> class span {
  public:
    /**
     * @brief SFINAE trait that detects whether `V` can be converted to a span.
     *
     * `EnableIfConvertibleFrom<V>` is a substitution-failure trait: it is `void` if `V` exposes a
     * convertible data pointer via `internal::HasData` and an integral `size()` via
     * `internal::HasSize`, and otherwise fails to substitute, disabling the overload that uses it.
     *
     * @tparam V The type to test.
     */
    template <typename V>
    using EnableIfConvertibleFrom =
        typename std::enable_if_t<internal::HasData<T, V>::value && internal::HasSize<V>::value>;

    /**
     * @brief SFINAE trait that is satisfied when `U` is a const view.
     *
     * Used to distinguish between const and mutable view constructors, so that a `const span<U>`
     * cannot be implicitly converted from a mutable view.
     *
     * @tparam U The view type to test.
     */
    template <typename U>
    using EnableIfConstView = typename std::enable_if_t<std::is_const_v<T>, U>;

    /**
     * @brief SFINAE trait that is satisfied when `U` is a mutable view.
     *
     * The complement of `EnableIfConstView` above, used for the mutable view constructors.
     *
     * @tparam U The view type to test.
     */
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

    /**
     * @brief Constructs an empty span.
     *
     * The resulting span points at `nullptr` with a size of zero.
     */
    SPECULA_CPU_GPU span() : data_(nullptr), size_(0) {}

    /**
     * @brief Constructs a span from a pointer and a length.
     *
     * @param ptr The first element of the sequence.
     * @param n The number of elements.
     */
    SPECULA_CPU_GPU span(T *ptr, size_type n) : data_(ptr), size_(n) {}

    /**
     * @brief Constructs a span from a C array.
     *
     * The size is deduced from the array's bound.
     *
     * @tparam N The number of elements in the array.
     * @param a The array to view.
     */
    template <std::size_t N> SPECULA_CPU_GPU span(T (&a)[N]) : span(a, N) {}

    /**
     * @brief Constructs a span from an initializer list.
     *
     * @param v The initializer list.
     */
    SPECULA_CPU_GPU span(std::initializer_list<value_type> v) : span(v.begin(), v.size()) {}

    /**
     * @brief Constructs a span from a mutable view-like object.
     *
     * SFINAE-constrained by `EnableIfConvertibleFrom<V>` and `EnableIfMutableView<V>`, so that a
     * mutable span can only be constructed from a mutable view.
     *
     * @tparam V The type of the view.
     * @param v The view to convert from.
     */
    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfMutableView<V>>
    SPECULA_CPU_GPU explicit span(V &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Constructs a span from a `std::vector`.
     *
     * @tparam V The element type of the vector.
     * @param v The vector to view.
     */
    template <typename V> span(std::vector<V> &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Constructs a span from a `std::vector`.
     *
     * @tparam V The element type of the vector.
     * @param v The vector to view.
     */
    template <typename V> span(const std::vector<V> &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Constructs a const span from any convertible view-like object.
     *
     * SFINAE-constrained by `EnableIfConvertibleFrom<V>` and `EnableIfConstView<V>`. This is the
     * only constructor that can construct a span of const elements.
     *
     * @tparam V The type of the view.
     * @param v The view to convert from.
     */
    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfConstView<V>>
    SPECULA_CPU_GPU constexpr span(const V &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Returns an iterator to the first element.
     *
     * Equivalent to `data()`.
     *
     * @return An iterator to the first element.
     */
    SPECULA_CPU_GPU iterator begin() { return data_; }

    /**
     * @brief Returns a const iterator to the first element.
     *
     * @return A const iterator to the first element.
     */
    SPECULA_CPU_GPU const_iterator begin() const { return data_; }

    /**
     * @brief Returns a const iterator to the first element.
     *
     * @return A const iterator to the first element.
     */
    SPECULA_CPU_GPU const_iterator cbegin() const { return data_; }

    /**
     * @brief Returns an iterator one past the last element.
     *
     * Equivalent to `data() + size()`.
     *
     * @return An iterator one past the last element.
     */
    SPECULA_CPU_GPU iterator end() { return data_ + size_; }

    /**
     * @brief Returns a const iterator one past the last element.
     *
     * @return A const iterator one past the last element.
     */
    SPECULA_CPU_GPU const_iterator end() const { return data_ + size_; }

    /**
     * @brief Returns a const iterator one past the last element.
     *
     * @return A const iterator one past the last element.
     */
    SPECULA_CPU_GPU const_iterator cend() const { return data_ + size_; }

    /**
     * @brief Returns the number of elements in the span.
     *
     * @return The number of elements.
     */
    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return size_; }

    /**
     * @brief Returns whether or not the span is empty.
     *
     * @return `true` if the span holds no elements.
     */
    SPECULA_CPU_GPU [[nodiscard]] bool empty() const { return size_ == 0; }

    /**
     * @brief Returns a reference to the element at the given index.
     *
     * The index is bounds-checked against `size()` using `DASSERT_LT`.
     *
     * @param index The index of the element.
     * @return A reference to the element at `index`.
     */
    SPECULA_CPU_GPU reference operator[](size_type index) {
      DASSERT_LT(index, size_);
      return data_[index];
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
      DASSERT_LT(index, size_);
      return data_[index];
    }

    /**
     * @brief Returns a const reference to the first element.
     *
     * @return A const reference to the first element.
     */
    SPECULA_CPU_GPU element_type front() const { return data_[0]; }

    /**
     * @brief Returns a const reference to the last element.
     *
     * @return A const reference to the last element.
     */
    SPECULA_CPU_GPU element_type back() const { return data_[size_ - 1]; }

    /**
     * @brief Returns a pointer to the first element.
     *
     * This is equivalent to `data()`. The pointer may be null when the span is empty.
     *
     * @return A pointer to the first element.
     */
    SPECULA_CPU_GPU pointer data() { return data_; }

    /**
     * @brief Returns a const pointer to the first element.
     *
     * @return A const pointer to the first element.
     */
    SPECULA_CPU_GPU const_pointer data() const { return data_; }

    /**
     * @brief Removes `count` elements from the front of the span.
     *
     * The data pointer is advanced by `count` and the size is reduced by the same amount. The
     * underlying memory is not modified.
     *
     * @param count The number of elements to remove.
     */
    SPECULA_CPU_GPU void remove_prefix(size_type count) {
      data_ += count;
      size_ -= count;
    }

    /**
     * @brief Removes `count` elements from the back of the span.
     *
     * Only the size is reduced; the data pointer is unchanged.
     *
     * @param count The number of elements to remove.
     */
    SPECULA_CPU_GPU void remove_suffix(size_type count) { size_ -= count; }

    /**
     * @brief Returns a subspan of the elements in the range `[pos, pos + count)`.
     *
     * The returned span starts at `data() + pos` and has length `count`, or the remaining number of
     * elements if `pos + count` would exceed the size of the span. If `count` is `dynamic_extent`
     * (the default), the whole remainder of the span is returned.
     *
     * @param pos The index of the first element of the subspan.
     * @param count The number of elements in the subspan. Defaults to `dynamic_extent`, meaning the
     * remainder.
     * @return A span containing the subspan's elements.
     */
    SPECULA_CPU_GPU span subspan(size_type pos, size_type count = dynamic_extent) {
      size_type np = count < (size() - pos) ? count : (size() - pos);
      return span(data_ + pos, np);
    }

  private:
    /**
     * @brief A pointer to the first element of the sequence.
     *
     * The span does not own this memory.
     */
    pointer data_;

    /**
     * @brief The number of elements in the sequence.
     *
     * The span does not own this memory.
     */
    size_type size_;
  };

  /**
   * @brief Creates a span from a pointer and a length.
   *
   * The `int &...ExplicitArgumentBarrier` parameter pack is a compiler hint that prevents the
   * function from being inlined into the caller, which improves inlining of the recursive
   * `make_span` overloads.
   *
   * @tparam T The element type of the span.
   * @param ptr The first element of the sequence.
   * @param size The number of elements.
   * @return A span over the given range.
   */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<T> make_span(T *ptr, size_t size) noexcept {
    return span<T>(ptr, size);
  }

  /**
   * @brief Creates a span from two pointers into the same sequence.
   *
   * The length is computed as `end - begin`.
   *
   * @tparam T The element type of the span.
   * @param begin The first element of the sequence.
   * @param end One past the last element of the sequence.
   * @return A span over the given range.
   */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU span<T> make_span(T *begin, T *end) noexcept {
    return span<T>(begin, end - begin);
  }

  /**
   * @brief Creates a span from a `std::vector`.
   *
   * @tparam T The element type of the vector.
   * @param v The vector to view.
   * @return A span over the vector's elements.
   */
  template <int &...ExplicitArgumentBarrier, typename T>
  span<T> make_span(std::vector<T> &v) noexcept {
    return span<T>(v.data(), v.size());
  }

  /**
   * @brief Creates a span from any container-like type.
   *
   * SFINAE-constrained by `internal::HasData` and `internal::HasSize`, so that any type exposing a
   * convertible `data()` and an integral `size()` is accepted. The `int
   * &...ExplicitArgumentBarrier` parameter pack prevents inlining of this recursive overload.
   *
   * @tparam C The type of the container.
   * @param c A reference to the container.
   * @return A span over the container's elements.
   */
  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU constexpr auto make_span(C &c) noexcept
      -> decltype(make_span(internal::get_data(c), c.size())) {
    return make_span(internal::get_data(c), c.size());
  }

  /**
   * @brief Creates a span from a C array.
   *
   * The size is deduced from the array's bound.
   *
   * @tparam T The element type of the array.
   * @tparam N The number of elements in the array.
   * @param array The array to view.
   * @return A span over the array's elements.
   */
  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU constexpr span<T> make_span(T (&array)[N]) noexcept {
    return span<T>(array, N);
  }

  /**
   * @brief Creates a const span from a pointer and a length.
   *
   * @tparam T The element type of the span.
   * @param ptr The first element of the sequence.
   * @param size The number of elements.
   * @return A const span over the given range.
   */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(T *ptr, size_t size) noexcept {
    return span<const T>(ptr, size);
  }

  /**
   * @brief Creates a const span from two pointers into the same sequence.
   *
   * The length is computed as `end - begin`.
   *
   * @tparam T The element type of the span.
   * @param begin The first element of the sequence.
   * @param end One past the last element of the sequence.
   * @return A const span over the given range.
   */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(T *begin, T *end) noexcept {
    return span<const T>(begin, end - begin);
  }

  /**
   * @brief Creates a const span from a `std::vector`.
   *
   * @tparam T The element type of the vector.
   * @param v The vector to view.
   * @return A const span over the vector's elements.
   */
  template <int &...ExplicitArgumentBarrier, typename T>
  constexpr span<const T> make_const_span(const std::vector<T> &v) noexcept {
    return span<const T>(v.data(), v.size());
  }

  /**
   * @brief Creates a const span from any const container-like type.
   *
   * Delegates to `make_span(c)` above, which in turn dispatches to the const container's `data()`
   * and `size()` members.
   *
   * @tparam C The type of the container.
   * @param c A const reference to the container.
   * @return A const span over the container's elements.
   */
  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU constexpr auto make_const_span(const C &c) noexcept -> decltype(make_span(c)) {
    return make_span(c);
  }

  /**
   * @brief Creates a const span from a C array.
   *
   * The size is deduced from the array's bound.
   *
   * @tparam T The element type of the array.
   * @tparam N The number of elements in the array.
   * @param array The array to view.
   * @return A const span over the array's elements.
   */
  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(const T (&array)[N]) noexcept {
    return span<const T>(array, N);
  }

} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_SPAN_HPP
