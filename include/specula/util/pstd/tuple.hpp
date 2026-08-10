#ifndef SPECULA_UTIL_PSTD_TUPLE_HPP
#define SPECULA_UTIL_PSTD_TUPLE_HPP

#include <cstddef>
#include <utility>

#include "specula/macros.hpp"

namespace specula::pstd {
  template <typename... Ts> struct tuple;

  template <> struct tuple<> {
    template <size_t> using type = void;
  };

  template <typename T, typename... Ts> struct tuple<T, Ts...> : tuple<Ts...> {
    using Base = tuple<Ts...>;

    tuple() = default;
    tuple(const tuple &) = default;
    tuple(tuple &&) = default;

    tuple(const T &value, const Ts &...values) : Base(values...), value(value) {}
    tuple(T &&value, Ts &&...values) : Base(std::move(values)...), value(std::move(value)) {}

    ~tuple() = default;

    tuple &operator=(tuple &&) = default;
    tuple &operator=(const tuple &) = default;

    T value;
  };

  template <typename... Ts> tuple(Ts &&...) -> tuple<std::decay_t<Ts>...>;

  template <size_t I, typename T, typename... Ts> SPECULA_CPU_GPU auto &get(tuple<T, Ts...> &t) {
    if constexpr (I == 0) {
      return t.value;
    } else {
      return get<I - 1>((tuple<Ts...> &)t);
    }
  }
  template <size_t I, typename T, typename... Ts>
  SPECULA_CPU_GPU const auto &get(const tuple<T, Ts...> &t) {
    if constexpr (I == 0) {
      return t.value;
    } else {
      return get<I - 1>((const tuple<Ts...> &)t);
    }
  }

  template <typename Req, typename T, typename... Ts>
  SPECULA_CPU_GPU auto &get(tuple<T, Ts...> &t) {
    if constexpr (std::is_same_v<Req, T>) {
      return t.value;
    } else {
      return get<Req>((tuple<Ts...> &)t);
    }
  }
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
