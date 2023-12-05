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

#  define CHECK_EQ(a, b) CHECK_IMPL(a, b, ==)
#  define CHECK_NE(a, b) CHECK_IMPL(a, b, !=)
#  define CHECK_GT(a, b) CHECK_IMPL(a, b, >)
#  define CHECK_GE(a, b) CHECK_IMPL(a, b, >=)
#  define CHECK_LT(a, b) CHECK_IMPL(a, b, <)
#  define CHECK_LE(a, b) CHECK_IMPL(a, b, <=)

#else

#  define CHECK(x) (!(!(x) && (LOG_CRITICAL("Check failed: {}", #x), true)))

#  define CHECK_IMPL(a, b, op)                                                                     \
    do {                                                                                           \
      auto va = a;                                                                                 \
      auto vb = b;                                                                                 \
      if (!(va op vb))                                                                             \
        LOG_CRITICAL("Check failed: {} " #op " {} with {} = {}, {} = {}", #a, #b, #a, va, #b, vb); \
    } while (false)

#  define CHECK_EQ(a, b) CHECK_IMPL(a, b, ==)
#  define CHECK_NE(a, b) CHECK_IMPL(a, b, !=)
#  define CHECK_GT(a, b) CHECK_IMPL(a, b, >)
#  define CHECK_GE(a, b) CHECK_IMPL(a, b, >=)
#  define CHECK_LT(a, b) CHECK_IMPL(a, b, <)
#  define CHECK_LE(a, b) CHECK_IMPL(a, b, <=)

#endif

#ifdef SPECULA_DEBUG_BUILD

#  define DCHECK(x) (CHECK(x))
#  define DCHECK_EQ(a, b) CHECK_EQ(a, b)
#  define DCHECK_NE(a, b) CHECK_NE(a, b)
#  define DCHECK_GT(a, b) CHECK_GT(a, b)
#  define DCHECK_GE(a, b) CHECK_GE(a, b)
#  define DCHECK_LT(a, b) CHECK_LT(a, b)
#  define DCHECK_LE(a, b) CHECK_LE(a, b)

#else

#  define EMPTY_CHECK                                                                              \
    do {                                                                                           \
    } while (false)

#  define DCHECK(x) EMPTY_CHECK

#  define DCHECK_EQ(a, b) EMPTY_CHECK
#  define DCHECK_NE(a, b) EMPTY_CHECK
#  define DCHECK_GT(a, b) EMPTY_CHECK
#  define DCHECK_GE(a, b) EMPTY_CHECK
#  define DCHECK_LT(a, b) EMPTY_CHECK
#  define DCHECK_LE(a, b) EMPTY_CHECK

#endif

} // namespace specula

#endif // !SPECULA_UTIL_CHECK_HPP
