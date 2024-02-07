/**
 * @file check.hpp
 * @brief Debug and release assertions
 * @date 2023-12-02
 * @copyright Copyright (c) 2023
 *
 * This file provides a set of macros for implementing assertions for debug and release mode. The
 * debug assertions are implemented using the standard `assert` macro, while the release assertions
 * are implemented using the `LOG_CRITICAL` macro from the logging utilities.
 */

#ifndef SPECULA_UTIL_CHECK_HPP
#define SPECULA_UTIL_CHECK_HPP

namespace specula {
#ifdef SPECULA_IS_GPU_CODE

#  define CHECK(x) assert(x)
#  define CHECK_IMPL(a, b, op) assert((a)op(b))

#else

  /** @brief Assertion macro for issuing fatal errors
   *
   * This macro should be used to replace the standard `assert()` as it will also log additional
   * information about the failure.
   *
   * @note When compiled for GPU, this macro becmes a standard `assert()`, and will not log any
   * error message.
   *
   * @param x Expression to check
   */
#  define CHECK(x) (!(!(x) && (LOG_CRITICAL("Check failed: {}", #x), true)))

  /**
   * @brief Implementation for value comparison assertions
   *
   * This macro is the implementation for the value comparison assertions `CHECK_*`. It takes care
   * to only evaluated the provided expressions once, to avoid potential side effects (e.g.
   * `var++`). And when the check fails, it will log a message included the source code form of the
   * check, and the values that caused the failure. And it is implemented as a single iteration
   * `do/while` loop, that way it is all contained as a single statement, and can be used in places
   * like `if` statements.
   *
   * @note When compiled for GPU, this macro becmes a standard `assert()`, and will not log any
   * error messages.
   *
   * @param a First expression to compare
   * @param b Second expression to compare
   * @param op Operator to use for comparison
   */
#  define CHECK_IMPL(a, b, op)                                                                     \
    do {                                                                                           \
      auto va = a;                                                                                 \
      auto vb = b;                                                                                 \
      if (!(va op vb))                                                                             \
        LOG_CRITICAL("Check failed: {} " #op " {} with {} = {}, {} = {}", #a, #b, #a, va, #b, vb); \
    } while (false)

#endif // SPECULA_IS_GPU_CODE

#define CHECK_EQ(a, b) CHECK_IMPL(a, b, ==)
#define CHECK_NE(a, b) CHECK_IMPL(a, b, !=)
#define CHECK_GT(a, b) CHECK_IMPL(a, b, >)
#define CHECK_GE(a, b) CHECK_IMPL(a, b, >=)
#define CHECK_LT(a, b) CHECK_IMPL(a, b, <)
#define CHECK_LE(a, b) CHECK_IMPL(a, b, <=)

#ifdef SPECULA_DEBUG_BUILD

#  define DCHECK(x) (CHECK(x))
#  define DCHECK_EQ(a, b) CHECK_EQ(a, b)
#  define DCHECK_NE(a, b) CHECK_NE(a, b)
#  define DCHECK_GT(a, b) CHECK_GT(a, b)
#  define DCHECK_GE(a, b) CHECK_GE(a, b)
#  define DCHECK_LT(a, b) CHECK_LT(a, b)
#  define DCHECK_LE(a, b) CHECK_LE(a, b)

#else

  /**
   * @brief Empty macro for release builds
   *
   * This macro is used to replace the debug assertions in release builds, and is defined as an
   * empty statement. That way, all the debug assertions are removed from the code, and the compiler
   * can optimize the code as if they were never there.
   */
#  define EMPTY_CHECK                                                                              \
    do {                                                                                           \
    } while (false)

  /**
   * @brief Check macro which is only available in debug builds
   *
   * In debug builds this will be the same as the `CHECK` macro, which evaluates the expression
   * and will log a critical error if it is false, but in release builds it will be be a no-op.
   *
   * @param x Expression to check
   *
   * @see EMPTY_CHECK, CHECK
   */
#  define DCHECK(x) EMPTY_CHECK

#  define DCHECK_EQ(a, b) EMPTY_CHECK
#  define DCHECK_NE(a, b) EMPTY_CHECK
#  define DCHECK_GT(a, b) EMPTY_CHECK
#  define DCHECK_GE(a, b) EMPTY_CHECK
#  define DCHECK_LT(a, b) EMPTY_CHECK
#  define DCHECK_LE(a, b) EMPTY_CHECK

#endif // SPECULA_DEBUG_BUILD

#define CHECK_RARE_TO_STRING(x) #x
#define CHECK_RARE_EXPAND_AND_TO_STRING(x) CHECK_RARE_TO_STRING(x)

#ifdef SPECULA_IS_GPU_CODE

#  define CHECK_RARE(freq, condition)
#  define DCHECK_RARE(freq, condition)

#else

  /**
   * @brief Check macro for conditions that allowed to happen rarely
   *
   * This macro is used to check conditions that are allowed to happen rarely, but where their
   * frequent occurrence would be a bug. The frequency of occurrences will be tracked, and reported
   * at the end of the renderer, along with the other statistics.
   *
   * @param freq The maximum permissible frequency at which the condition should be checked
   * @param condition The condition to check
   *
   * @see stats.hpp
   */
#  define CHECK_RARE(freq, condition)                                                              \
    static_assert(std::is_floating_point<decltype(freq)>::value,                                   \
                  "Expected floating-point frequency as first argoument to CHECK_RARE");           \
    static_assert(std::is_integral<decltype(condition)>::value,                                    \
                  "Expected Boolean condition as second argument to CHECK_RARE");                  \
    do {                                                                                           \
      static thread_local int64_t num_true, total;                                                 \
      static StatRegisterer reg([](StatsAccumulator &accum) {                                      \
        accum.report_rare_check(__FILE__ " " CHECK_RARE_EXPAND_AND_TO_STRING(                      \
                                    __LINE__) ": CHECK_RARE failed: " #condition,                  \
                                freq, num_true, total);                                            \
        num_true = total = 0;                                                                      \
      });                                                                                          \
      ++total;                                                                                     \
      if (condition)                                                                               \
        ++num_true;                                                                                \
    } while (false)

#  ifdef SPECULA_DEBUG_BUILD
#    define DCHECK_RARE(freq, condition) CHECK_RARE(freq, condition)
#  else
#    define DCHECK_RARE(freq, condition)
#  endif // SPECULA_DEBUG_BUILD

#endif // SPECULA_IS_GPU_CODE

} // namespace specula

#endif // !SPECULA_UTIL_CHECK_HPP
