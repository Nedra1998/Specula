/**
 * @file
 * @brief A lightweight, variadic tuple type usable on both the host (CPU) and device (GPU)
 *
 * This header defines `specula::pstd::tuple`, a fixed-size tuple whose storage is the concatenation
 * of the storage of its element types. It is a minimal re-implementation of the subset of
 * `std::tuple` needed by the renderer: construction, default/copy/move operations, decayed type
 * deduction and element access via `get`.
 */

#ifndef SPECULA_UTIL_PSTD_TUPLE_HPP
#define SPECULA_UTIL_PSTD_TUPLE_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <cstddef>
#include <utility>

#include "specula/macros.hpp"

namespace specula::pstd {
  /**
   * @brief A lightweight, variadic tuple type usable on both the host (CPU) and device (GPU)
   *
   * `tuple` stores its elements contiguously, with the first template parameter being the type of
   * the first element and so on. The class derives from a base tuple containing the remaining
   * elements, which lets all of the constructors and the `get<I>` accessors be implemented by
   * simple delegation down to the single-element case.
   *
   * The primary template is only ever instantiated through the partial specialisation for the empty
   * tuple below, so a tuple of zero elements is a distinct, default-constructible type with no
   * storage.
   *
   * @tparam Ts The types of the elements of the tuple, in order.
   */
  template <typename... Ts> struct tuple;

  /**
   * @brief The specialisation of `tuple` for an empty tuple.
   *
   * Provides `type<I>` as `void` for any index `I`, which is the empty tuple's analogue of
   * `std::tuple<>::type<I>`. It is default-constructible and holds no data.
   */
  template <> struct tuple<> {
    /**
     * @brief The type of the element at index `I`.
     *
     * Always `void` for the empty tuple.
     *
     * @tparam I The index of the element.
     */
    template <size_t> using type = void;
  };

  /**
   * @brief The partial specialisation of `tuple` for a tuple of one or more elements.
   *
   * Derives from `tuple<Ts...>` (the tuple without its first element) and adds a member `value` of
   * type `T`, which is the first element of the tuple.
   *
   * @tparam T The type of the first element.
   * @tparam Ts The types of the remaining elements.
   */
  template <typename T, typename... Ts> struct tuple<T, Ts...> : tuple<Ts...> {
    /**
     * @brief The type of the base tuple.
     *
     * @see tuple<Ts...>
     */
    using Base = tuple<Ts...>;

    /**
     * @brief Default constructor.
     *
     * Value-initialises the first element and default-constructs the rest.
     */
    tuple() = default;

    /**
     * @brief Copy constructor.
     *
     * Delegated to the compiler-generated copy constructor.
     */
    tuple(const tuple &) = default;

    /**
     * @brief Move constructor.
     *
     * Delegated to the compiler-generated move constructor.
     */
    tuple(tuple &&) = default;

    /**
     * @brief Constructs a tuple from a value for each element.
     *
     * The first argument value-initialises `value`, the remaining arguments are forwarded to the
     * base tuple's constructor.
     *
     * @param value The value of the first element.
     * @param values The values of the remaining elements.
     */
    tuple(const T &value, const Ts &...values) : Base(values...), value(value) {}

    /**
     * @brief Constructs a tuple from a value for each element.
     *
     * The first argument value-initialises `value`, the remaining arguments are forwarded to the
     * base tuple's constructor. This overload is preferred when the arguments are rvalues.
     *
     * @param value The value of the first element.
     * @param values The values of the remaining elements.
     */
    tuple(T &&value, Ts &&...values) : Base(std::move(values)...), value(std::move(value)) {}

    /**
     * @brief Destructor.
     *
     * Delegated to the compiler-generated destructor.
     */
    ~tuple() = default;

    /**
     * @brief Move assignment operator.
     *
     * Delegated to the compiler-generated move assignment operator.
     */
    tuple &operator=(tuple &&) = default;

    /**
     * @brief Copy assignment operator.
     *
     * Delegated to the compiler-generated copy assignment operator.
     */
    tuple &operator=(const tuple &) = default;

    /**
     * @brief The value of the first element.
     *
     * This is the only member of the specialisation, and is accessed through the `get` functions
     * below.
     */
    T value;
  };

  /**
   * @brief Deduction guide for `tuple`.
   *
   * Deduces the tuple's element types from the argument types, decayed. So a call `tuple(1, 2.0)`
   * deduces `tuple<int, double>`.
   */
  template <typename... Ts> tuple(Ts &&...) -> tuple<std::decay_t<Ts>...>;

  /**
   * @brief Returns a reference to the element at index `I`.
   *
   * Specialises for the first element, in which case it simply returns `t.value`. Otherwise it
   * reinterprets `t` as a tuple without its first element and recurses with `I - 1`. This is
   * equivalent to `std::get<I>(t)` for the tuple's own element types.
   *
   * @tparam I The index of the element.
   * @tparam T The type of the first element.
   * @tparam Ts The types of the remaining elements.
   * @param t The tuple to access.
   * @return A reference to the element at index `I`.
   */
  template <size_t I, typename T, typename... Ts> SPECULA_CPU_GPU auto &get(tuple<T, Ts...> &t) {
    if constexpr (I == 0) {
      return t.value;
    } else {
      return get<I - 1>((tuple<Ts...> &)t);
    }
  }

  /**
   * @brief Returns a const reference to the element at index `I`.
   *
   * The const overload mirrors the non-const one above. Note that the reinterpreted cast produces a
   * const tuple, so the const recursion terminates at the const `get` overload.
   *
   * @tparam I The index of the element.
   * @tparam T The type of the first element.
   * @tparam Ts The types of the remaining elements.
   * @param t The tuple to access.
   * @return A const reference to the element at index `I`.
   */
  template <size_t I, typename T, typename... Ts>
  SPECULA_CPU_GPU const auto &get(const tuple<T, Ts...> &t) {
    if constexpr (I == 0) {
      return t.value;
    } else {
      return get<I - 1>((const tuple<Ts...> &)t);
    }
  }

  /**
   * @brief Returns a reference to the element of a given type.
   *
   * The template parameter `Req` is the expected type of the element to return. If `Req` matches
   * the first element's type `T` exactly, the member is returned directly; otherwise the tuple is
   * reinterpreted as a tuple without its first element and the search recurses with `Req`.
   *
   * This is the `std::get<Req>(t)` overload used, for example, to obtain a reference to an element
   * whose static type differs from the tuple's declared element types.
   *
   * @tparam Req The expected type of the element.
   * @tparam T The type of the first element.
   * @tparam Ts The types of the remaining elements.
   * @param t The tuple to access.
   * @return A reference to the element of type `Req`.
   */
  template <typename Req, typename T, typename... Ts>
  SPECULA_CPU_GPU auto &get(tuple<T, Ts...> &t) {
    if constexpr (std::is_same_v<Req, T>) {
      return t.value;
    } else {
      return get<Req>((tuple<Ts...> &)t);
    }
  }

  /**
   * @brief Returns a const reference to the element of a given type.
   *
   * The const counterpart of the overload above, with identical semantics.
   *
   * @tparam Req The expected type of the element.
   * @tparam T The type of the first element.
   * @tparam Ts The types of the remaining elements.
   * @param t The tuple to access.
   * @return A const reference to the element of type `Req`.
   */
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
