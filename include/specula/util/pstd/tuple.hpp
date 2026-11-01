/**
 * @file
 * @brief Minimal GPU-safe replacement for `std::tuple`.
 *
 * pbrt cannot use `std::tuple` in device code, so this header provides the small subset it needs:
 * element-wise construction, CTAD and `get` by index or by type. It is built by recursive
 * inheritance: `tuple<T, Ts...>` derives from `tuple<Ts...>` and stores `T` in a `value` member.
 *
 * Not provided (compared to `std::tuple`): `tuple_size` / `tuple_element` (so no structured
 * bindings), comparison operators, `make_tuple` / `tie` / `apply`, rvalue `get`, converting
 * constructors and `constexpr` support.
 *
 * @see https://en.cppreference.com/w/cpp/utility/tuple
 */

#ifndef SPECULA_UTIL_PSTD_TUPLE_HPP
#define SPECULA_UTIL_PSTD_TUPLE_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cstddef>
#include <utility>

#include "specula/macros.hpp"

namespace specula::pstd {
  /**
   * @brief Fixed-size heterogeneous collection; GPU-safe subset of `std::tuple`.
   *
   * Only the two specialisations below are defined.
   */
  template <typename... Ts> struct tuple;

  /** @brief Empty tuple: the base case of the recursive inheritance; holds no data. */
  template <> struct tuple<> {
    /**
     * @brief Always `void`. Not part of `std::tuple`; kept from pbrt.
     * @todo Not used by the non-empty specialisation; possibly dead.
     */
    template <size_t> using type = void;
  };

  /**
   * @brief Non-empty tuple: stores the first element in `value` and inherits the rest.
   *
   * Element `0` is the most-derived `value`; element `I` lives in the `I`-th base.
   */
  template <typename T, typename... Ts> struct tuple<T, Ts...> : tuple<Ts...> {
    /** @brief The tuple of the remaining elements. */
    using Base = tuple<Ts...>;

    /**
     * @brief Default-initialises every element.
     * @warning Unlike `std::tuple`, scalar elements are left indeterminate (not value-initialised)
     * when the tuple is default-initialised.
     */
    tuple() = default;
    /** @brief Copies each element. */
    tuple(const T &value, const Ts &...values) : Base(values...), value(value) {}
    /**
     * @brief Moves each element.
     * @note Chosen only when *every* argument is an rvalue of the exact element type; mixed
     * lvalue/rvalue arguments fall back to the copying overload.
     */
    tuple(T &&value, Ts &&...values) : Base(std::move(values)...), value(std::move(value)) {}
    /** @brief Defaulted copy constructor. */
    tuple(const tuple &) = default;
    /** @brief Defaulted move constructor. */
    tuple(tuple &&) = default;

    /** @brief Defaulted destructor. */
    ~tuple() = default;

    /** @brief Defaulted copy assignment. */
    tuple &operator=(const tuple &) = default;
    /** @brief Defaulted move assignment. */
    tuple &operator=(tuple &&) = default;

    /** @brief The first element; use `pstd::get` instead of accessing this directly. */
    T value;
  };

  /** @brief Deduces decayed element types, e.g. `tuple(1, 2.0)` is `tuple<int, double>`. */
  template <typename... Ts> tuple(Ts &&...) -> tuple<std::decay_t<Ts>...>;

  /**
   * @brief Returns the element at index `I`.
   *
   * Recurses by upcasting `t` to its base tuple and decrementing `I`.
   *
   * @pre `I < sizeof...(Ts) + 1`; an out-of-range index fails to compile with an unhelpful error
   * rather than a `static_assert`.
   * @see https://en.cppreference.com/w/cpp/utility/tuple/get
   */
  template <size_t I, typename T, typename... Ts> SPECULA_CPU_GPU auto &get(tuple<T, Ts...> &t) {
    if constexpr (I == 0) {
      return t.value;
    } else {
      return get<I - 1>((tuple<Ts...> &)t);
    }
  }

  /** @brief Returns the element at index `I` (const overload). */
  template <size_t I, typename T, typename... Ts>
  SPECULA_CPU_GPU const auto &get(const tuple<T, Ts...> &t) {
    if constexpr (I == 0) {
      return t.value;
    } else {
      return get<I - 1>((const tuple<Ts...> &)t);
    }
  }

  /**
   * @brief Returns the first element whose type is exactly `Req`.
   *
   * @warning Deviates from `std::get<T>`: if `Req` occurs more than once, the first match is
   * returned instead of the call being ill-formed. If `Req` does not occur, compilation fails deep
   * in the recursion (no `tuple<>` overload).
   * @see https://en.cppreference.com/w/cpp/utility/tuple/get
   */
  template <typename Req, typename T, typename... Ts>
  SPECULA_CPU_GPU auto &get(tuple<T, Ts...> &t) {
    if constexpr (std::is_same_v<Req, T>) {
      return t.value;
    } else {
      return get<Req>((tuple<Ts...> &)t);
    }
  }

  /** @brief Returns the first element whose type is exactly `Req` (const overload). */
  template <typename Req, typename T, typename... Ts>
  SPECULA_CPU_GPU const auto &get(const tuple<T, Ts...> &t) {
    if constexpr (std::is_same_v<Req, T>) {
      return t.value;
    } else {
      return get<Req>((const tuple<Ts...> &)t);
    }
  }
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_TUPLE_HPP
