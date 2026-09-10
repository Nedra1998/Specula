#ifndef SPECULA_UTIL_CONTAINERS_TYPE_PACK_HPP
#define SPECULA_UTIL_CONTAINERS_TYPE_PACK_HPP

#include <cstddef>
#include <type_traits>

namespace specula {
  template <typename... Ts> struct TypePack {
    static constexpr size_t count = sizeof...(Ts);
  };

  template <typename T, typename... Ts> struct IndexOf {
    static constexpr size_t count = 0;
    static_assert(!std::is_same_v<T, T>, "Type not present in TypePack");
  };

  template <typename T, typename... Ts> struct IndexOf<T, TypePack<T, Ts...>> {
    static constexpr size_t count = 0;
  };

  template <typename T, typename U, typename... Ts> struct IndexOf<T, TypePack<U, Ts...>> {
    static constexpr size_t count = 1 + IndexOf<T, TypePack<Ts...>>::count;
  };

  template <typename T, typename... Ts> struct HasType {
    static constexpr bool value = false;
  };

  template <typename T, typename U, typename... Ts> struct HasType<T, TypePack<U, Ts...>> {
    static constexpr bool value = (std::is_same_v<T, U> || HasType<T, TypePack<Ts...>>::value);
  };

  template <typename T> struct GetFirst {};

  template <typename T, typename... Ts> struct GetFirst<TypePack<T, Ts...>> {
    using type = T;
  };

  template <typename T> struct RemoveFirst {};

  template <typename T, typename... Ts> struct RemoveFirst<TypePack<T, Ts...>> {
    using type = TypePack<Ts...>;
  };

  template <size_t INDEX, typename T, typename... Ts> struct RemoveFirstN {};
  template <size_t INDEX, typename T, typename... Ts>
  struct RemoveFirstN<INDEX, TypePack<T, Ts...>> {
    using type = typename RemoveFirstN<INDEX - 1, TypePack<Ts...>>::type;
  };
  template <typename T, typename... Ts> struct RemoveFirstN<0, TypePack<T, Ts...>> {
    using type = TypePack<T, Ts...>;
  };

  template <typename... Ts> struct Prepend {};
  template <typename T, typename... Ts> struct Prepend<T, TypePack<Ts...>> {
    using type = TypePack<T, Ts...>;
  };
  template <typename... Ts> struct Prepend<void, TypePack<Ts...>> {
    using type = TypePack<Ts...>;
  };

  template <size_t INDEX, typename T, typename... Ts> struct TakeFirstN {};
  template <size_t INDEX, typename T, typename... Ts> struct TakeFirstN<INDEX, TypePack<T, Ts...>> {
    using type = typename Prepend<T, typename TakeFirstN<INDEX - 1, TypePack<Ts...>>::type>::type;
  };
  template <typename T, typename... Ts> struct TakeFirstN<1, TypePack<T, Ts...>> {
    using type = TypePack<T>;
  };

  template <template <typename> class M, typename... Ts> struct MapType;
  template <template <typename> class M, typename T> struct MapType<M, TypePack<T>> {
    using type = TypePack<M<T>>;
  };
  template <template <typename> class M, typename T, typename... Ts>
  struct MapType<M, TypePack<T, Ts...>> {
    using type = typename Prepend<M<T>, typename MapType<M, TypePack<Ts...>>::type>::type;
  };

  template <typename Base, typename... Ts> inline constexpr bool all_inherit_from(TypePack<Ts...>);
  template <typename Base> inline constexpr bool all_inherit_from(TypePack<> /*unused*/) {
    return true;
  }
  template <typename Base, typename T, typename... Ts>
  inline constexpr bool all_inherit_from(TypePack<T, Ts...> /*unused*/) {
    return std::is_base_of_v<Base, T> && all_inherit_from<Base>(TypePack<Ts...>());
  }

  template <typename F, typename... Ts> void for_each_type(F func, TypePack<Ts...>);
  template <typename F, typename T, typename... Ts>
  void for_each_type(F func, TypePack<T, Ts...> /*unused*/) {
    func.template operator()<T>();
    for_each_type(func, TypePack<Ts...>());
  }
  template <typename F> void for_each_type(F func, TypePack<> /*unused*/) {}

} // namespace specula

#endif // SPECULA_UTIL_CONTAINERS_TYPE_PACK_HPP
