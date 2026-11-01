/**
 * @file
 * @brief Device-safe optional value, a stand-in for `std::optional`, plus its `fmt` formatter.
 */

#ifndef SPECULA_UTIL_PSTD_OPTIONAL_HPP
#define SPECULA_UTIL_PSTD_OPTIONAL_HPP

// IWYU pragma: private, include "specula/util/pstd.hpp"

#include <new>
#include <type_traits>
#include <utility>

#include <fmt/base.h>

#include "specula/macros.hpp"
#include "specula/util/check.hpp"

namespace specula::pstd {
  /**
   * @brief Holds either a `T` or nothing, usable on both CPU and GPU.
   *
   * Port of pbrt's `pstd::optional`: `std::optional` cannot be relied on in GPU device code. The
   * value lives in raw aligned storage and is constructed with placement `new`. Differences from
   * `std::optional`:
   * - accessing an empty optional fails an `ASSERT` instead of throwing `std::bad_optional_access`;
   * - moving from an optional leaves the source **empty** (`std` leaves it holding a moved-from
   *   value);
   * - there is no `nullopt`, `emplace`, `swap`, comparison operators or monadic operations, and
   *   nothing is `constexpr`;
   * - copy/move are never trivial, even for trivially copyable `T`.
   *
   * @see https://en.cppreference.com/w/cpp/utility/optional
   */
  template <typename T> class optional {
  public:
    using value_type = T;

    /** @brief Constructs an empty optional. */
    optional() = default;

    /** @brief Constructs an optional holding a copy of `v` (implicit, like `std`). */
    SPECULA_CPU_GPU optional(const T &v) : set(true) { new (ptr()) T(v); }
    /** @brief Constructs an optional holding `v` moved in. */
    SPECULA_CPU_GPU optional(T &&v) : set(true) { new (ptr()) T(std::move(v)); }
    /** @brief Copies the contained value of `v`, if any. */
    SPECULA_CPU_GPU optional(const optional &v) : set(v.has_value()) {
      if (v.has_value()) {
        new (ptr()) T(v.value());
      }
    }
    /**
     * @brief Moves the contained value of `v`, if any, then empties `v`.
     * @todo `noexcept` is unconditional; `std` uses `std::is_nothrow_move_constructible_v<T>`.
     */
    SPECULA_CPU_GPU optional(optional &&v) noexcept : set(v.has_value()) {
      if (v.has_value()) {
        new (ptr()) T(std::move(v.value()));
        v.reset();
      }
    }

    /** @brief Destroys the contained value, if any. */
    SPECULA_CPU_GPU ~optional() { reset(); }

    /** @brief Destroys any current value, then stores a copy of `v`. */
    SPECULA_CPU_GPU optional &operator=(const T &v) {
      reset();
      new (ptr()) T(v);
      set = true;
      return *this;
    }
    /** @brief Destroys any current value, then stores `v` moved in. */
    SPECULA_CPU_GPU optional &operator=(T &&v) {
      reset();
      new (ptr()) T(std::move(v));
      set = true;
      return *this;
    }
    /**
     * @brief Destroys any current value, then copies the value of `v`, if any.
     * @todo Self-assignment is UB: `reset()` destroys the value before it is copied.
     */
    SPECULA_CPU_GPU optional &operator=(const optional &v) {
      reset();
      if (v.has_value()) {
        new (ptr()) T(v.value());
        set = true;
      }
      return *this;
    }
    /**
     * @brief Destroys any current value, then moves the value of `v`, if any, and empties `v`.
     * @todo Self-assignment is UB, and `noexcept` is unconditional (see the move constructor).
     */
    SPECULA_CPU_GPU optional &operator=(optional &&v) noexcept {
      reset();
      if (v.has_value()) {
        new (ptr()) T(std::move(v.value()));
        set = true;
        v.reset();
      }
      return *this;
    }

    /** @brief `true` when a value is held; same as `has_value()`. */
    SPECULA_CPU_GPU explicit operator bool() const { return set; }

    /** @brief `true` when a value is held. */
    SPECULA_CPU_GPU [[nodiscard]] bool has_value() const { return set; }

    /** @brief The contained value. @pre `has_value()`, checked with `ASSERT` (not an exception). */
    SPECULA_CPU_GPU T &value() {
      ASSERT(set);
      return *ptr();
    }
    /** @brief The contained value. @pre `has_value()`, checked with `ASSERT` (not an exception). */
    SPECULA_CPU_GPU const T &value() const {
      ASSERT(set);
      return *ptr();
    }
    /** @brief A copy of the contained value, or `alt` when empty. */
    SPECULA_CPU_GPU T value_or(const T &alt) const { return set ? value() : alt; }

    /** @brief Pointer to the contained value. @pre `has_value()` (`ASSERT`ed). */
    SPECULA_CPU_GPU T *operator->() { return &value(); }
    /** @brief Pointer to the contained value. @pre `has_value()` (`ASSERT`ed). */
    SPECULA_CPU_GPU const T *operator->() const { return &value(); }
    /** @brief The contained value. @pre `has_value()` (`ASSERT`ed). */
    SPECULA_CPU_GPU T &operator*() { return value(); }
    /** @brief The contained value. @pre `has_value()` (`ASSERT`ed). */
    SPECULA_CPU_GPU const T &operator*() const { return value(); }

    /** @brief Destroys the contained value, if any, leaving the optional empty. */
    SPECULA_CPU_GPU void reset() {
      if (set) {
        value().~T();
        set = false;
      }
    }

  private:
    /**
     * @brief Typed pointer into the raw storage.
     *
     * `std::launder` is required to legally reach the object placement-`new`ed there; NVCC builds
     * skip it (as pbrt does).
     */
#ifdef __NVCC__
    SPECULA_CPU_GPU T *ptr() { return reinterpret_cast<T *>(&optional_value); }
    SPECULA_CPU_GPU const T *ptr() const { return reinterpret_cast<const T *>(&optional_value); }
#else
    SPECULA_CPU_GPU T *ptr() { return std::launder(reinterpret_cast<T *>(&optional_value)); }
    SPECULA_CPU_GPU const T *ptr() const {
      return std::launder(reinterpret_cast<const T *>(&optional_value));
    }
#endif

    /**
     * @brief Uninitialized storage for the value; only live while `set` is `true`.
     * @todo `std::aligned_storage_t` is deprecated in C++23; use `alignas(T) std::byte[sizeof(T)]`.
     */
    std::aligned_storage_t<sizeof(T), alignof(T)> optional_value;
    /** @brief Whether `optional_value` holds a constructed `T`. */
    bool set = false;
  };
} // namespace specula::pstd

/**
 * @brief Formats a `pstd::optional<T>` as `optional(<value>)`, or `none` when empty.
 *
 * Requires a `fmt::formatter<T>`. Format specs are not supported.
 */
template <typename T> struct fmt::formatter<specula::pstd::optional<T>> {
  /** @brief Accepts only an empty format spec. */
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  /** @brief Writes `optional(<value>)` or `none`. */
  template <typename FormatContext>
  auto format(const specula::pstd::optional<T> &v, FormatContext &ctx) const {
    if (v.has_value()) {
      return format_to(ctx.out(), "optional({})", v.value());
    }
    return format_to(ctx.out(), "none");
  }
};

#endif // SPECULA_UTIL_PSTD_OPTIONAL_HPP
