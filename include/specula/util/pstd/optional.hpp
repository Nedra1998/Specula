/**
 * @file optional.hpp
 * @brief Portable optional value class
 *
 * This file provides a portable optional value class that is compatible with both the CPU and GPU.
 */

#ifndef SPECULA_UTIL_PSTD_OPTIONAL_HPP_
#define SPECULA_UTIL_PSTD_OPTIONAL_HPP_

#include "specula/specula.hpp"
#include "specula/util/check.hpp"

namespace specula::pstd {
  /**
   * @class optional
   * @tparam T The type of the optional value.
   *
   * An optional value class that reimplements `std::optional` so that it can be used on both the
   * CPU and GPU. For details on how to use this class, refer to the C++ standard library.
   */
  template <typename T> class optional {
  public:
    using value_type = T;

    /**
     * @brief Create an empty optional.
     */
    optional() = default;

    /**
     * @brief Create an optional with a value.
     *
     * @param v Initial value of the optional.
     */
    SPECULA_CPU_GPU optional(const T &v) : set(true) { new (ptr()) T(v); }

    /**
     * @brief Create an optional with a value.
     *
     * @param v Initial value of the optional.
     */
    SPECULA_CPU_GPU optional(T &&v) : set(true) { new (ptr()) T(std::move(v)); }

    /**
     * @brief Copy existing optional.
     *
     * @param v Optional value to copy.
     */
    SPECULA_CPU_GPU optional(const optional &v) : set(v.has_value()) {
      if (v.has_value())
        new (ptr()) T(v.value());
    }

    /**
     * @brief Copy existing optional.
     *
     * @param v Optional value to copy.
     */
    SPECULA_CPU_GPU optional(optional &&v) : set(v.has_value()) {
      if (v.has_value()) {
        new (ptr()) T(std::move(v.value()));
        v.reset();
      }
    }

    /**
     * @brief Destroy the optional value.
     *
     * Destroy the optional value and call the destructor of the contained value.
     */
    SPECULA_CPU_GPU ~optional() { reset(); }

    /**
     * @brief Assign a value to the optional.
     *
     * @param v Value to assign to the optional.
     * @return The optional value with the new value assigned.
     */
    SPECULA_CPU_GPU optional &operator=(const T &v) {
      reset();
      new (ptr()) T(v);
      set = true;
      return *this;
    }

    /**
     * @brief Assign a value to the optional.
     *
     * @param v The value to assign to the optional.
     * @return The optional value with the new value assigned.
     */
    SPECULA_CPU_GPU optional &operator=(T &&v) {
      reset();
      new (ptr()) T(std::move(v));
      set = true;
      return *this;
    }

    /**
     * @brief Assign an optional value to the optional.
     *
     * @param v Other optional value to assign to the optional.
     * @return The optional value with the new value assigned.
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
     * @brief Assign an optional value to the optional.
     *
     * @param v other optional value to assign to the optional.
     * @return The optional value with the new value assigned.
     */
    SPECULA_CPU_GPU optional &operator=(optional &&v) {
      reset();
      if (v.has_value()) {
        new (ptr()) T(std::move(v.value()));
        set = true;
        v.reset();
      }
      return *this;
    }

    /// @brief Check if the optional value is set.
    SPECULA_CPU_GPU explicit operator bool() const { return set; }

    /// @brief Access the optional value.
    SPECULA_CPU_GPU T *operator->() { return &value(); }

    /// @brief Access the optional value.
    SPECULA_CPU_GPU const T *operator->() const { return &value(); }

    /// @brief Access the optional value.
    SPECULA_CPU_GPU T &operator*() { return value(); }

    /// @brief Access the optional value.
    SPECULA_CPU_GPU const T &operator*() const { return value(); }

    /**
     * @brief Get the value of the optional or an alternative value.
     *
     * @param alt Alternative value to return if the optional is not set.
     * @return The value of the optional or the alternative value if the optional is not set.
     */
    SPECULA_CPU_GPU inline T value_or(const T &alt) const { return set ? value() : alt; }

    /**
     * @brief Get the value of the optional.
     *
     * @warning This function will assert if the optional is not set. So ensure that the optional is
     * set before calling this function.
     *
     * @return The value of the optional.
     */
    SPECULA_CPU_GPU inline T &value() {
      ASSERT(set);
      return *ptr();
    }

    /**
     * @brief Get the value of the optional.
     *
     * @warning This function will assert if the optional is not set. So ensure that the optional is
     * set before calling this function.
     *
     * @return The value of the optional.
     */
    SPECULA_CPU_GPU inline const T &value() const {
      ASSERT(set);
      return *ptr();
    }

    /**
     * @brief Reset the optional value.
     *
     * Destroy the optional value, calling the destructor of the value. And set the `set` flag to
     * false.
     */
    SPECULA_CPU_GPU void reset() {
      if (set) {
        value().~T();
        set = false;
      }
    }

    /**
     * @brief Check if the optional value is set.
     *
     * @return True if the optional value is set, false otherwise.
     */
    SPECULA_CPU_GPU inline bool has_value() const { return set; }

  private:
#ifdef __NVCC__
    SPECULASPECULA_CPU_GPU T *ptr() { return reinterpret_cast<T *>(&optional_value); }
    SPECUSPECULA_CPU_GPU const T *ptr() const {
      return reinterpret_cast<const T *>(&optional_value);
    }
#else
    SPECULA_CPU_GPU T *ptr() { return reinterpret_cast<T *>(&optional_value); }
    SPECULA_CPU_GPU const T *ptr() const {
      return std::launder(reinterpret_cast<const T *>(&optional_value));
    }
#endif // __NVCC__

    std::aligned_storage_t<sizeof(T), alignof(T)> optional_value;
    bool set = false;
  };
} // namespace specula::pstd

#endif // SPECULA_UTIL_PSTD_OPTIONAL_HPP_
