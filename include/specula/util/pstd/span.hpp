/**
 * @file
 * @brief `pstd::span`: a GPU-safe, non-owning view over contiguous memory, plus `make_span`
 * helpers.
 *
 * A trimmed-down port of pbrt's `pstd::span` (itself adapted from Abseil's `absl::Span`). It
 * exists because `std::span` is not usable in device code; every member is `SPECULA_CPU_GPU`
 * except the `std::vector` conversions, which are host-only.
 *
 * Differences from `std::span`:
 * - dynamic extent only (no `Extent` template parameter);
 * - `front()`/`back()` return by value, not by reference;
 * - adds `remove_prefix()`/`remove_suffix()` (as on `std::string_view`);
 * - no `first()`, `last()`, `size_bytes()`, reverse iterators or `as_bytes()`.
 *
 * @see https://en.cppreference.com/w/cpp/container/span
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
  /**
   * @brief Count value meaning "everything up to the end" for `span::subspan()`.
   *
   * Equal to `std::numeric_limits<std::size_t>::max()` (written as `-1`, as in pbrt).
   *
   * @see https://en.cppreference.com/w/cpp/container/span/dynamic_extent
   */
  inline constexpr std::size_t dynamic_extent = -1;

  /** @brief Detection helpers for `span`'s converting constructors and `make_span`. */
  namespace internal {
    /**
     * @brief Returns `c.data()`; removed from overload resolution if `C` has no `data()`.
     *
     * The unused `char` parameter is the Abseil overload-ranking idiom (`get_data` passes an
     * `int`, so an `int` overload would win over this one). Only this overload exists here.
     */
    template <typename C>
    SPECULA_CPU_GPU constexpr auto get_data_impl(C &c, char /*unused*/) noexcept
        -> decltype(c.data()) {
      return c.data();
    }

    /** @brief SFINAE-friendly `c.data()`, used in trailing return types. */
    template <typename C>
    SPECULA_CPU_GPU constexpr auto get_data(C &c) noexcept -> decltype(get_data_impl(c, 0)) {
      return get_data_impl(c, 0);
    }

    /**
     * @brief `std::true_type` if `C::size()` returns an integral type.
     *
     * A substitution failure (rather than `false`) when `C` has no `size()`.
     */
    template <typename C>
    using HasSize = std::is_integral<typename std::decay_t<decltype(std::declval<C &>().size())>>;

    /**
     * @brief `std::true_type` if `C::data()` yields a pointer usable as a `T *`.
     *
     * Tests @f$ U^{**} \to T^{*}\,\mathrm{const}^{*} @f$ convertibility (one more level of
     * indirection than needed), which only allows qualification conversions such as
     * `int *` -> `const int *`, never derived-to-base pointer conversions that would break
     * pointer arithmetic.
     */
    template <typename T, typename C>
    using HasData =
        std::is_convertible<typename std::decay_t<decltype(get_data(std::declval<C &>()))> *,
                            T *const *>;
  } // namespace internal

  /**
   * @brief Non-owning view of `size()` contiguous `T`s starting at `data()`.
   *
   * Copying a span copies the pointer, not the elements. Writes through a `span<T>` modify the
   * viewed memory; use `span<const T>` for read-only views. The span dangles if the underlying
   * buffer is freed or reallocated (e.g. a `std::vector` growing).
   *
   * @see https://en.cppreference.com/w/cpp/container/span
   */
  template <typename T> class span {
  public:
    using element_type = T;
    using value_type = typename std::remove_cv_t<T>;
    using size_type = std::size_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;

    /** @brief `void` if `V` has a `data()` convertible to `T *` and an integral `size()`. */
    template <typename V>
    using EnableIfConvertibleFrom =
        typename std::enable_if_t<internal::HasData<T, V>::value && internal::HasSize<V>::value>;

    /**
     * @brief `U` if this is a span of const elements, otherwise a substitution failure.
     * @note The condition tests this span's `T`, not `U`; `U` is only the result type.
     */
    template <typename U>
    using EnableIfConstView = typename std::enable_if_t<std::is_const_v<T>, U>;

    /** @brief `U` if this is a span of mutable elements; complement of `EnableIfConstView`. */
    template <typename U>
    using EnableIfMutableView = typename std::enable_if_t<!std::is_const_v<T>, U>;

    /** @brief Empty span: `data() == nullptr`, `size() == 0`. */
    SPECULA_CPU_GPU span() : data_(nullptr), size_(0) {}

    /** @brief Views `n` elements starting at `ptr`. */
    SPECULA_CPU_GPU span(T *ptr, size_type n) : data_(ptr), size_(n) {}

    /** @brief Views a C array; the size is deduced from its bound. */
    template <std::size_t N> SPECULA_CPU_GPU span(T (&a)[N]) : span(a, N) {}

    /**
     * @brief Views the elements of an initializer list.
     *
     * Only compiles for `span<const T>`, since the list's elements are const.
     *
     * @warning The list's backing array dies at the end of the full-expression, so this is only
     * safe for a span passed directly as a function argument, e.g. `f({1, 2, 3})`.
     */
    SPECULA_CPU_GPU span(std::initializer_list<value_type> v) : span(v.begin(), v.size()) {}

    /**
     * @brief Views a `std::vector` (host only).
     * @note Unconstrained: a `std::vector<V>` whose `V *` is not convertible to `T *` is a hard
     * error rather than an overload-resolution failure.
     */
    template <typename V> span(std::vector<V> &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Views a const `std::vector` (host only).
     * @note Only compiles for `span<const T>`; same unconstrained caveat as above.
     */
    template <typename V> span(const std::vector<V> &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Mutable span from any mutable container with `data()` and `size()`.
     *
     * Enabled only when `T` is non-const. `explicit` so that handing a container to a function
     * taking a mutable span is a visible decision (pbrt/Abseil behaviour).
     */
    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfMutableView<V>>
    SPECULA_CPU_GPU explicit span(V &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Const span from any container with `data()` and `size()`.
     *
     * Enabled only when `T` is const; implicit, so containers convert freely to read-only views.
     */
    template <typename V, typename X = EnableIfConvertibleFrom<V>,
              typename Y = EnableIfConstView<V>>
    SPECULA_CPU_GPU constexpr span(const V &v) noexcept : span(v.data(), v.size()) {}

    /**
     * @brief Element at `index`.
     * @pre `index < size()` (checked with `DASSERT_LT` in debug builds only).
     */
    SPECULA_CPU_GPU reference operator[](size_type index) {
      DASSERT_LT(index, size_);
      return data_[index];
    }

    /**
     * @brief Element at `index`.
     * @pre `index < size()` (checked with `DASSERT_LT` in debug builds only).
     */
    SPECULA_CPU_GPU const_reference operator[](size_type index) const {
      DASSERT_LT(index, size_);
      return data_[index];
    }

    /**
     * @brief Copy of the first element.
     * @pre `!empty()` (unchecked).
     * @note Returns by value, unlike `std::span::front()` which returns a reference (pbrt
     * behaviour).
     */
    SPECULA_CPU_GPU element_type front() const { return data_[0]; }

    /**
     * @brief Copy of the last element.
     * @pre `!empty()` (unchecked).
     * @note Returns by value, unlike `std::span::back()` (pbrt behaviour).
     */
    SPECULA_CPU_GPU element_type back() const { return data_[size_ - 1]; }

    /** @brief Pointer to the first element; may be `nullptr` when empty. */
    SPECULA_CPU_GPU pointer data() { return data_; }

    /** @brief Pointer to the first element; may be `nullptr` when empty. */
    SPECULA_CPU_GPU const_pointer data() const { return data_; }

    /** @brief Iterator to the first element. */
    SPECULA_CPU_GPU iterator begin() { return data_; }

    /** @brief Iterator to the first element. */
    SPECULA_CPU_GPU const_iterator begin() const { return data_; }

    /** @brief Iterator one past the last element. */
    SPECULA_CPU_GPU iterator end() { return data_ + size_; }

    /** @brief Iterator one past the last element. */
    SPECULA_CPU_GPU const_iterator end() const { return data_ + size_; }

    /** @brief Const iterator to the first element. */
    SPECULA_CPU_GPU const_iterator cbegin() const { return data_; }

    /** @brief Const iterator one past the last element. */
    SPECULA_CPU_GPU const_iterator cend() const { return data_ + size_; }

    /** @brief Number of elements. */
    SPECULA_CPU_GPU [[nodiscard]] size_type size() const { return size_; }

    /** @brief Whether `size() == 0`. */
    SPECULA_CPU_GPU [[nodiscard]] bool empty() const { return size_ == 0; }

    /**
     * @brief Drops the first `count` elements from the view (the memory is untouched).
     * @pre `count <= size()` (unchecked).
     */
    SPECULA_CPU_GPU void remove_prefix(size_type count) {
      data_ += count;
      size_ -= count;
    }

    /**
     * @brief Drops the last `count` elements from the view (the memory is untouched).
     * @pre `count <= size()` (unchecked).
     */
    SPECULA_CPU_GPU void remove_suffix(size_type count) { size_ -= count; }

    /**
     * @brief View of the elements in @f$ [pos, pos + \min(count, size() - pos)) @f$.
     *
     * Unlike `std::span::subspan()`, a `count` that runs past the end is clamped rather than UB,
     * so the default `dynamic_extent` simply means "to the end".
     *
     * @pre `pos <= size()` (unchecked; `size() - pos` would wrap around).
     * @todo Should be `const`, as `std::span::subspan()` is.
     */
    SPECULA_CPU_GPU span subspan(size_type pos, size_type count = dynamic_extent) {
      size_type np = count < (size() - pos) ? count : (size() - pos);
      return span(data_ + pos, np);
    }

  private:
    pointer data_;   ///< First viewed element (not owned).
    size_type size_; ///< Number of viewed elements.
  };

  // `make_span` / `make_const_span` deduce the element type from their argument, like
  // `absl::MakeSpan`. The leading `int &...ExplicitArgumentBarrier` pack can never be deduced or
  // explicitly specified, which forces callers to rely on deduction (`make_span<float>(p, n)` is
  // ill-formed). The pointer-and-size overloads come first because the container overloads use
  // them in their trailing return types.

  /** @brief Span over `size` elements starting at `ptr`. */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<T> make_span(T *ptr, size_t size) noexcept {
    return span<T>(ptr, size);
  }

  /** @brief Span over the pointer range `[begin, end)`. */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU span<T> make_span(T *begin, T *end) noexcept {
    return span<T>(begin, end - begin);
  }

  /** @brief Span over a `std::vector` (host only). */
  template <int &...ExplicitArgumentBarrier, typename T>
  span<T> make_span(std::vector<T> &v) noexcept {
    return span<T>(v.data(), v.size());
  }

  /**
   * @brief Span over any container with `data()` and `size()`.
   *
   * The element type follows `c.data()`, so a const container yields a `span<const T>`.
   */
  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU constexpr auto make_span(C &c) noexcept
      -> decltype(make_span(internal::get_data(c), c.size())) {
    return make_span(internal::get_data(c), c.size());
  }

  /** @brief Span over a C array; the size is deduced from its bound. */
  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU constexpr span<T> make_span(T (&array)[N]) noexcept {
    return span<T>(array, N);
  }

  /** @brief Read-only span over `size` elements starting at `ptr`. */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(T *ptr, size_t size) noexcept {
    return span<const T>(ptr, size);
  }

  /** @brief Read-only span over the pointer range `[begin, end)`. */
  template <int &...ExplicitArgumentBarrier, typename T>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(T *begin, T *end) noexcept {
    return span<const T>(begin, end - begin);
  }

  /** @brief Read-only span over a `std::vector` (host only). */
  template <int &...ExplicitArgumentBarrier, typename T>
  constexpr span<const T> make_const_span(const std::vector<T> &v) noexcept {
    return span<const T>(v.data(), v.size());
  }

  /** @brief Read-only span over any container with `data()` and `size()`. */
  template <int &...ExplicitArgumentBarrier, typename C>
  SPECULA_CPU_GPU constexpr auto make_const_span(const C &c) noexcept -> decltype(make_span(c)) {
    return make_span(c);
  }

  /** @brief Read-only span over a C array; the size is deduced from its bound. */
  template <int &...ExplicitArgumentBarrier, typename T, size_t N>
  SPECULA_CPU_GPU constexpr span<const T> make_const_span(const T (&array)[N]) noexcept {
    return span<const T>(array, N);
  }

} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_SPAN_HPP
