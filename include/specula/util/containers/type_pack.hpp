#ifndef INCLUDE_CONTAINERS_TYPE_PACK_HPP_
#define INCLUDE_CONTAINERS_TYPE_PACK_HPP_

#include <cstddef>
#include <type_traits>

namespace specula {
  template <typename... Ts> struct TypePack {
    static constexpr size_t count = sizeof...(Ts);
  };

  template <typename T, typename... Ts> struct IndexOf {
    static constexpr int count = 0;
    static_assert(!std::is_same_v<T, T>, "Type not found in TypePack");
  };

  template <typename T, typename... Ts> struct IndexOf<T, TypePack<T, Ts...>> {
    static constexpr int count = 0;
  };

  template <typename T, typename U, typename... Ts> struct IndexOf<T, TypePack<U, Ts...>> {
    static constexpr int count = 1 + IndexOf<T, TypePack<Ts...>>::count;
  };

} // namespace specula

#endif // INCLUDE_CONTAINERS_TYPE_PACK_HPP_
