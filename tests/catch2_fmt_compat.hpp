/**
 * @file catch2_fmt_compat.hpp
 * @brief Bridges {fmt} formatters into Catch2's stringification pipeline.
 *
 * Include this header once (e.g., in main.cpp or a shared test helper) before
 * any TEST_CASE definitions.  After inclusion, any type that has a
 * `fmt::formatter<T>` specialisation will automatically be rendered by Catch2
 * using `fmt::format("{}", value)` — no `operator<<` overload required.
 *
 * Catch2 stringifies values through `Catch::StringMaker<T>`.  The primary
 * template falls back to `operator<<` when available, or emits "{?}" for
 * unknown types.  We provide a *partial specialisation* gated on
 * `fmt::is_formattable<T>` that intercepts any type with a registered fmt
 * formatter before Catch2 reaches its unprintable-string path.
 *
 * The `!Catch::Detail::IsStreamInsertable_v<T>` guard prevents ambiguity:
 * types that already have an `operator<<` will continue to use it (matching
 * Catch2's default behaviour), while types that only have a fmt formatter will
 * be routed through this specialisation.
 */

#pragma once

#include <string>
#include <type_traits>

#include <catch2/catch_tostring.hpp>
#include <fmt/format.h>

namespace Catch {

  /// Generic StringMaker for any type that has a fmt::formatter<T> but does
  /// NOT have an operator<< (to avoid ambiguity with Catch2's default path).
  template <typename T>
  struct StringMaker<
      T, std::enable_if_t<fmt::is_formattable<T>::value && !Detail::IsStreamInsertable_v<T>>> {
    static std::string convert(T const &value) { return fmt::format("{}", value); }
  };

} // namespace Catch
