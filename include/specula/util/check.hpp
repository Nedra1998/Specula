#ifndef SPECULA_CHECK_HPP
#define SPECULA_CHECK_HPP

#include <functional>
#include <string>
#include <vector>

#include "specula/util/log.hpp"

namespace specula {
#ifdef SPECULA_IS_GPU_CODE
#  define ASSERT(x) assert(x)
#  define ASSERT_IMPL(a, b, op) assert((a)op(b))
#else
  // TODO: Assertion fails should also trigger CheckCallbackScope::fail();
#  define ASSERT(x) (!(!(x) && (LOG_CRITICAL("Check failed: {}", #x), true)))

#  define ASSERT_IMPL(a, b, op)                                                                    \
    do {                                                                                           \
      auto va = a;                                                                                 \
      auto vb = b;                                                                                 \
      if (!(va op vb)) {                                                                           \
        LOG_CRITICAL("Check failed: {} " #op " {} with {} = {}, {} = {}", #a, #b, #a, va, #b, vb); \
        CheckCallbackScope::fail();                                                                \
      }                                                                                            \
    } while (false)
#endif

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
#  define EMPTY_ASSERT                                                                             \
    do {                                                                                           \
    } while (false)

#  define DASSERT(x) EMPTY_ASSERT

#  define DASSERT_EQ(a, b) EMPTY_ASSERT
#  define DASSERT_NE(a, b) EMPTY_ASSERT
#  define DASSERT_GT(a, b) EMPTY_ASSERT
#  define DASSERT_GE(a, b) EMPTY_ASSERT
#  define DASSERT_LT(a, b) EMPTY_ASSERT
#  define DASSERT_LE(a, b) EMPTY_ASSERT

#endif

#define ASSERT_RARE_TO_STRING(x) #x
#define ASSERT_RARE_EXPAND_AND_TO_STRING(x) ASSERT_RARE_TO_STRING(x)

#ifdef SPECULA_IS_GPU_CODE

#  define ASSERT_RARE(freq, condition)
#  define DASSERT_RARE(freq, condition)

#else

#  define ASSERT_RARE(freq, condition)                                                             \
    static_assert(std::is_floating_point<decltype(freq)>::value,                                   \
                  "Expected floating-point frequency as first argument to ASSERT_RARE");           \
    static_assert(std::is_integral<decltype(condition)>::value,                                    \
                  "Expected boolean condition as second argument to ASSERT_RARE");                 \
    do {                                                                                           \
      static thread_local int64_t num_true, total;                                                 \
      static StatRegisterer reg([](StatsAccumulator &accum) {                                      \
        accum.report_rare_check(__FILE__ " " ASSERT_RARE_EXPAND_AND_TO_STRING(                     \
                                    __LINE__) ": ASSERT_RARE failed: " #condition,                 \
                                frreq, num_true, total);                                           \
        num_true = total = 0;                                                                      \
      });                                                                                          \
      ++total;                                                                                     \
      if (condition) {                                                                             \
        ++num_true;                                                                                \
      }                                                                                            \
    } while (false)

#  ifdef SPECULA_DEBUG_BUILD
#    define DASSERT_RARE(freq, condition) ASSERT_RARE(freq, condition)
#  else
#    define DASSERT_RARE(freq, condition)
#  endif // SPECULA_DEBUG_BUILD

#endif

  class CheckCallbackScope {
  public:
    CheckCallbackScope(const std::function<std::string(void)> &callback);
    CheckCallbackScope(const CheckCallbackScope &) = delete;
    CheckCallbackScope(CheckCallbackScope &&) = delete;
    ~CheckCallbackScope();

    CheckCallbackScope &operator=(const CheckCallbackScope &) = delete;
    CheckCallbackScope &operator=(CheckCallbackScope &&) = delete;

    static void fail();

  private:
    static std::vector<std::function<std::string(void)>> callbacks;
  };

  void log_stack_trace();

} // namespace specula

#endif // SPECULA_CHECK_HPP
