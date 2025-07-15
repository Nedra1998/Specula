#ifndef INCLUDE_MATH_COMPENSATED_HPP_
#define INCLUDE_MATH_COMPENSATED_HPP_

#include <fmt/format.h>

#include "specula/specula.hpp"

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

    Float sum = 0, c = 0;
  };

  struct CompensatedFloat {
  public:
    SPECULA_CPU_GPU CompensatedFloat(Float v, Float err = 0) : v(v), err(err) {}
    SPECULA_CPU_GPU explicit operator float() const { return v + err; }
    SPECULA_CPU_GPU explicit operator double() const { return double(v) + double(err); }

    Float v, err;
  };
} // namespace specula

template <typename T> struct fmt::formatter<specula::CompensatedSum<T>> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::CompensatedSum<T> &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ CompensatedSum sum={} c={} ]", v.sum, v.c);
  }
};

template <> struct fmt::formatter<specula::CompensatedFloat> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::CompensatedFloat &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ CompensatedFloat v={} err={} ]", v.v, v.err);
  }
};

#endif // INCLUDE_MATH_COMPENSATED_HPP_
