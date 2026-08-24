#ifndef SPECULA_UTIL_PSTD_OPTIONAL_HPP
#define SPECULA_UTIL_PSTD_OPTIONAL_HPP

#include <new>
#include <type_traits>
#include <utility>

#include <fmt/base.h>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"

namespace specula::pstd {
  /**
   * @brief A container for an optional value
   *
   * @tparam T The type of the value
   *
   * @see https://en.cppreference.com/cpp/utility/optional
   */
  template <typename T> class optional {
  public:
    using value_type = T;

    optional() = default;

    SPECULA_CPU_GPU optional(const T &v) : set(true) { new (ptr()) T(v); }
    SPECULA_CPU_GPU optional(T &&v) : set(true) { new (ptr()) T(std::move(v)); }
    SPECULA_CPU_GPU optional(const optional &v) : set(v.has_value()) {
      if (v.has_value()) {
        new (ptr()) T(v.value());
      }
    }
    SPECULA_CPU_GPU optional(optional &&v) noexcept : set(v.has_value()) {
      if (v.has_value()) {
        new (ptr()) T(std::move(v.value()));
        v.reset();
      }
    }

    SPECULA_CPU_GPU ~optional() { reset(); }

    SPECULA_CPU_GPU optional &operator=(const T &v) {
      reset();
      new (ptr()) T(v);
      set = true;
      return *this;
    }
    SPECULA_CPU_GPU optional &operator=(T &&v) {
      reset();
      new (ptr()) T(std::move(v));
      set = true;
      return *this;
    }
    SPECULA_CPU_GPU optional &operator=(const optional &v) {
      reset();
      if (v.has_value()) {
        new (ptr()) T(v.value());
        set = true;
      }
      return *this;
    }
    SPECULA_CPU_GPU optional &operator=(optional &&v) noexcept {
      reset();
      if (v.has_value()) {
        new (ptr()) T(std::move(v.value()));
        set = true;
        v.reset();
      }
      return *this;
    }

    SPECULA_CPU_GPU [[nodiscard]] bool has_value() const { return set; }

    SPECULA_CPU_GPU T &value() {
      ASSERT(set);
      return *ptr();
    }
    SPECULA_CPU_GPU const T &value() const {
      ASSERT(set);
      return *ptr();
    }
    SPECULA_CPU_GPU T value_or(const T &alt) const { return set ? value() : alt; }

    SPECULA_CPU_GPU void reset() {
      if (set) {
        value().~T();
        set = false;
      }
    }

    SPECULA_CPU_GPU explicit operator bool() const { return set; }

    SPECULA_CPU_GPU T *operator->() { return &value(); }
    SPECULA_CPU_GPU const T *operator->() const { return &value(); }
    SPECULA_CPU_GPU T &operator*() { return value(); }
    SPECULA_CPU_GPU const T &operator*() const { return value(); }

  private:
#ifdef __NVCC__
    SPECULA_CPU_GPU T *ptr() { return reinterpret_cast<T *>(&optional_value); }
    SPECULA_CPU_GPU const T *ptr() const { return reinterpret_cast<const T *>(&optional_value); }
#else
    SPECULA_CPU_GPU T *ptr() { return std::launder(reinterpret_cast<T *>(&optional_value)); }
    SPECULA_CPU_GPU const T *ptr() const {
      return std::launder(reinterpret_cast<const T *>(&optional_value));
    }
#endif

    std::aligned_storage_t<sizeof(T), alignof(T)> optional_value;
    bool set = false;
  };
} // namespace specula::pstd

template <typename T> struct fmt::formatter<specula::pstd::optional<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::pstd::optional<T> &v, FormatContext &ctx) const {
    if (v.has_value()) {
      return format_to(ctx.out(), "optional({})", v.value());
    }
    return format_to(ctx.out(), "none");
  }
};

#endif // SPECULA_UTIL_PSTD_OPTIONAL_HPP
