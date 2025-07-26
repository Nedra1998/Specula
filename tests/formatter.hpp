#ifndef INCLUDE_TESTS_FORMATTER_HPP_
#define INCLUDE_TESTS_FORMATTER_HPP_

#include <string>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_tostring.hpp>
#include <fmt/core.h>

namespace Catch {

  // Helper trait to detect if a type T is formattable by fmt::format
  template <typename T, typename = void> struct is_fmt_formattable : std::false_type {};

  template <typename T>
  struct is_fmt_formattable<T, std::void_t<decltype(fmt::format("{}", std::declval<T const &>()))>>
      : std::true_type {};

  template <typename T> struct StringMaker<T> {
    static std::string convert(const T &value) {
      if constexpr (is_fmt_formattable<T>::value) {
        return fmt::format("{}", value);
      } else {
        static_assert(
            is_fmt_formattable<T>::value,
            "T must be formattable using fmt::format for this StringMaker specialization.");
        return {}; // never reached
      }
    }
  };

} // namespace Catch

#endif // INCLUDE_TESTS_FORMATTER_HPP_
