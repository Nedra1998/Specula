#ifndef SPECULA_UTIL_MATH_COMPENSATED_HPP
#define SPECULA_UTIL_MATH_COMPENSATED_HPP

#include <type_traits>

#include <fmt/base.h>

#include "specula/macros.hpp"
#include "specula/types.hpp"
#include "specula/util/float.hpp"

namespace specula {
  template <typename Float> class CompensatedSum {
  public:
    CompensatedSum() = default;
    SPECULA_CPU_GPU explicit CompensatedSum(Float v) : sum(v) {}
    SPECULA_CPU_GPU CompensatedSum &operator=(Float v) {
      sum = v;
      c = 0;
      return *this;
    }

    SPECULA_CPU_GPU CompensatedSum &operator+=(Float v) {
      Float delta = v - c;
      Float new_sum = sum + delta;
      c = (new_sum - sum) - delta;
      sum = new_sum;
      return *this;
    }

    SPECULA_CPU_GPU explicit operator Float() const { return sum; }

  private:
    Float sum = 0, c = 0;

    friend struct fmt::formatter<CompensatedSum<Float>>;
  };

  struct CompensatedFloat {
  public:
    SPECULA_CPU_GPU CompensatedFloat(Float v, Float err = 0) : v(v), err(err) {}
    SPECULA_CPU_GPU explicit operator float() const { return v + err; }
    SPECULA_CPU_GPU explicit operator double() const { return double(v) + double(err); }

    Float v, err;
  };

  SPECULA_CPU_GPU inline CompensatedFloat two_prod(Float a, Float b) {
    Float ab = a * b;
    return {ab, fma(a, b, -ab)};
  }
  SPECULA_CPU_GPU inline CompensatedFloat two_sum(Float a, Float b) {
    Float s = a + b, delta = s - a;
    return {s, (a - (s - delta)) + (b - delta)};
  }

  namespace internal {
    template <typename Float>
    SPECULA_CPU_GPU inline CompensatedFloat inner_product(Float a, Float b) {
      return two_prod(a, b);
    }

    template <typename Float, typename... T>
    SPECULA_CPU_GPU inline CompensatedFloat inner_product(Float a, Float b, T... terms) {
      CompensatedFloat ab = two_prod(a, b);
      CompensatedFloat tp = inner_product(terms...);
      CompensatedFloat sum = two_sum(ab.v, tp.v);
      return {sum.v, ab.err + (tp.err + sum.err)};
    }
  } // namespace internal

  template <typename... T>
  SPECULA_CPU_GPU inline Float inner_product(T... terms)
    requires(std::conjunction_v<std::is_arithmetic<T>...>)
  {
    CompensatedFloat ip = internal::inner_product(terms...);
    return Float(ip);
  }

} // namespace specula

template <typename T> struct fmt::formatter<specula::CompensatedSum<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::CompensatedSum<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ CompensatedSum sum={} c={} ]", v.sum, v.c);
  }
};

template <> struct fmt::formatter<specula::CompensatedFloat> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const specula::CompensatedFloat &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ CompensatedFloat v={} err={} ]", v.v, v.err);
  }
};

#endif // SPECULA_UTIL_MATH_COMPENSATED_HPP
