/**
 * @file check.hpp
 * @brief Debug and release assertions
 *
 * This file provides a set of macros for implementing assertions for debug and release mode. The
 * debug assertions are implemented using the standard `assert` macro, while the release assertions
 * are implemented using the `LOG_CRITICAL` macro from the logging utilities.
 */

#ifndef SPECULA_UTIL_ASSERT_HPP
#define SPECULA_UTIL_ASSERT_HPP

#include <functional>
#include <string>
#include <vector>

#include "specula/util/log.hpp"

namespace specula {
#ifdef SPECULA_IS_GPU_CODE

#  define ASSERT(x) assert(x)
#  define ASSERT_IMPL(a, b, op) assert((a)op(b))

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
#  define ASSERT(x) (!(!(x) && (LOG_CRITICAL("Check failed: {}", #x), true)))

  /**
   * @brief Implementation for value comparison assertions
   *
   * This macro is the implementation for the value comparison assertions `ASSERT_*`. It takes care
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
#  define ASSERT_IMPL(a, b, op)                                                                    \
    do {                                                                                           \
      auto va = a;                                                                                 \
      auto vb = b;                                                                                 \
      if (!(va op vb)) {                                                                           \
        LOG_CRITICAL("Check failed: {} " #op " {} with {} = {}, {} = {}", #a, #b, #a, va, #b, vb); \
        CheckCallbackScope::fail();                                                                \
      }                                                                                            \
    } while (false)

#endif // SPECULA_IS_GPU_CODE

#define ASSERT_EQ(a, b) ASSERT_IMPL(a, b, ==)
#define ASSERT_NE(a, b) ASSERT_IMPL(a, b, !=)
#define ASSERT_GT(a, b) ASSERT_IMPL(a, b, >)
#define ASSERT_GE(a, b) ASSERT_IMPL(a, b, >=)
#define ASSERT_LT(a, b) ASSERT_IMPL(a, b, <)
#define ASSERT_LE(a, b) ASSERT_IMPL(a, b, <=)

#ifdef SPECULA_DEBUG_BUILD

#  define DASSERT(x) (ASSERT(x))
#  define DASSERT_EQ(a, b) ASSERT_EQ(a, b)
#  define DASSERT_NE(a, b) ASSERT_NE(a, b)
#  define DASSERT_GT(a, b) ASSERT_GT(a, b)
#  define DASSERT_GE(a, b) ASSERT_GE(a, b)
#  define DASSERT_LT(a, b) ASSERT_LT(a, b)
#  define DASSERT_LE(a, b) ASSERT_LE(a, b)

#else

  /**
   * @brief Empty macro for release builds
   *
   * This macro is used to replace the debug assertions in release builds, and is defined as an
   * empty statement. That way, all the debug assertions are removed from the code, and the compiler
   * can optimize the code as if they were never there.
   */
#  define EMPTY_ASSERT                                                                             \
    do {                                                                                           \
    } while (false)

  /**
   * @brief Check macro which is only available in debug builds
   *
   * In debug builds this will be the same as the `ASSERT` macro, which evaluates the expression
   * and will log a critical error if it is false, but in release builds it will be be a no-op.
   *
   * @param x Expression to check
   *
   * @see EMPTY_ASSERT, ASSERT
   */
#  define DASSERT(x) EMPTY_ASSERT

#  define DASSERT_EQ(a, b) EMPTY_ASSERT
#  define DASSERT_NE(a, b) EMPTY_ASSERT
#  define DASSERT_GT(a, b) EMPTY_ASSERT
#  define DASSERT_GE(a, b) EMPTY_ASSERT
#  define DASSERT_LT(a, b) EMPTY_ASSERT
#  define DASSERT_LE(a, b) EMPTY_ASSERT

#endif // SPECULA_DEBUG_BUILD

#define ASSERT_RARE_TO_STRING(x) #x
#define ASSERT_RARE_EXPAND_AND_TO_STRING(x) ASSERT_RARE_TO_STRING(x)

#ifdef SPECULA_IS_GPU_CODE

#  define ASSERT_RARE(freq, condition)
#  define DASSERT_RARE(freq, condition)

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
#  define ASSERT_RARE(freq, condition)                                                             \
    static_assert(std::is_floating_point<decltype(freq)>::value,                                   \
                  "Expected floating-point frequency as first argoument to ASSERT_RARE");          \
    static_assert(std::is_integral<decltype(condition)>::value,                                    \
                  "Expected Boolean condition as second argument to ASSERT_RARE");                 \
    do {                                                                                           \
      static thread_local int64_t num_true, total;                                                 \
      static StatRegisterer reg([](StatsAccumulator &accum) {                                      \
        accum.report_rare_check(__FILE__ " " ASSERT_RARE_EXPAND_AND_TO_STRING(                     \
                                    __LINE__) ": ASSERT_RARE failed: " #condition,                 \
                                freq, num_true, total);                                            \
        num_true = total = 0;                                                                      \
      });                                                                                          \
      ++total;                                                                                     \
      if (condition)                                                                               \
        ++num_true;                                                                                \
    } while (false)

#  ifdef SPECULA_DEBUG_BUILD
#    define DASSERT_RARE(freq, condition) ASSERT_RARE(freq, condition)
#  else
#    define DASSERT_RARE(freq, condition)
#  endif // SPECULA_DEBUG_BUILD

#endif // SPECULA_IS_GPU_CODE

  /**
   * @class CheckCallbackScope
   * @brief Scoped callback for custom check failure handling
   *
   * This class provides a scoped callback for custom check failure handling. It can be used to
   * register a callback function that will be called when a check fails, and the scope will ensure
   * that the callback is unregistered when it goes out of scope.
   */
  class CheckCallbackScope {
  public:
    /**
     * @brief Construct a new Check Callback Scope object
     *
     * @param callback The callback function to register.
     */
    CheckCallbackScope(std::function<std::string(void)> callback);
    /**
     * @brief Deregister the check callback function
     */
    ~CheckCallbackScope();

    CheckCallbackScope(const CheckCallbackScope &) = delete;
    CheckCallbackScope &operator=(const CheckCallbackScope &) = delete;

    /**
     * @brief Handle a check failure
     *
     * This function will be called when a check fails, and will print a stack trace and call all
     * the registered callbacks.
     */
    static void fail();

  private:
    static std::vector<std::function<std::string(void)>> callbacks;
  };

  /**
   * @brief Log the current stack trace as an error message
   *
   * This function checks the `SPECULA_BACKTRACE` environment variable, and if it is unset or set to
   * `0` it does nothing. If it is set to `1` it will print the stack trace, and if it is set to `2`
   * or `full` it will print the stack trace with code snippets.
   *
   * @todo This is currently implemented using the
   * [cpptrace](https://github.com/jeremy-rifkin/cpptrace) library, reimplement this function using
   * [`std::stacktrace`](https://en.cppreference.com/w/cpp/header/stacktrace) when it is more widly
   * supported, to reduce the number of dependencies and improve portability.
   *
   * @param force If true, the stack trace will be printed even if the `SPECULA_BACKTRACE`
   * environment variable is not set.
   */
  void print_stack_trace(bool force = false);

} // namespace specula

#endif // !SPECULA_UTIL_ASSERT_HPP
