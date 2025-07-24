#ifndef INCLUDE_MATH_INTERVAL_HPP_
#define INCLUDE_MATH_INTERVAL_HPP_

#include "specula/specula.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math/constants.hpp"
#include "specula/util/math/functions.hpp"

namespace specula {
  class Interval {
  public:
#ifndef SPECULA_IS_GPU_CODE
    static const Interval PI;
#endif // SPECULA_IS_GPU_CODE

    Interval() = default;

    SPECULA_CPU_GPU explicit Interval(Float v) : low(v), high(v) {}
    SPECULA_CPU_GPU constexpr Interval(Float low, Float high)
        : low(std::min(low, high)), high(std::max(low, high)) {}

    SPECULA_CPU_GPU static Interval from_value_and_error(Float value, Float error) {
      Interval i;
      if (error == 0) {
        i.low = i.high = value;
      } else {
        i.low = sub_round_down(value, error);
        i.high = add_round_up(value, error);
      }
      return i;
    }

    SPECULA_CPU_GPU Interval &operator=(Float v) {
      low = high = v;
      return *this;
    }

    SPECULA_CPU_GPU Float upper_bound() const { return high; }
    SPECULA_CPU_GPU Float lower_bound() const { return low; }
    SPECULA_CPU_GPU Float midpoint() const { return (low + high) / 2; }
    SPECULA_CPU_GPU Float width() const { return high - low; }

    SPECULA_CPU_GPU Float operator[](int i) const {
      DASSERT(i == 0 || i == 1);
      return i == 0 ? low : high;
    }

    SPECULA_CPU_GPU explicit operator Float() const { return midpoint(); }
    SPECULA_CPU_GPU bool exactly(Float v) const { return low == v && high == v; }
    SPECULA_CPU_GPU bool operator==(Float v) const { return exactly(v); }
    SPECULA_CPU_GPU Interval operator-() const { return {-high, -low}; }
    SPECULA_CPU_GPU Interval operator+(Interval i) const {
      return {add_round_down(low, i.low), add_round_up(high, i.high)};
    }
    SPECULA_CPU_GPU Interval operator-(Interval i) const {
      return {sub_round_down(low, i.high), sub_round_up(high, i.low)};
    }

    SPECULA_CPU_GPU Interval operator*(Interval i) const {
      Float lp[4] = {
          mul_round_down(low, i.low),
          mul_round_down(high, i.low),
          mul_round_down(low, i.high),
          mul_round_down(high, i.high),
      };
      Float hp[4] = {
          mul_round_up(low, i.low),
          mul_round_up(high, i.low),
          mul_round_up(low, i.high),
          mul_round_up(high, i.high),
      };

      return {std::min({lp[0], lp[1], lp[2], lp[3]}), std::max({hp[0], hp[1], hp[2], hp[3]})};
    }

    SPECULA_CPU_GPU Interval operator/(Interval i) const;
    SPECULA_CPU_GPU bool operator==(Interval i) const { return low == i.low && high == i.high; }

    SPECULA_CPU_GPU bool operator!=(Float f) const { return f < low || f > high; }

    SPECULA_CPU_GPU Interval &operator+=(Interval i) {
      *this = Interval(*this + i);
      return *this;
    }

    SPECULA_CPU_GPU Interval &operator-=(Interval i) {
      *this = Interval(*this - i);
      return *this;
    }

    SPECULA_CPU_GPU Interval &operator*=(Interval i) {
      *this = Interval(*this * i);
      return *this;
    }

    SPECULA_CPU_GPU Interval &operator/=(Interval i) {
      *this = Interval(*this / i);
      return *this;
    }

    SPECULA_CPU_GPU Interval &operator+=(Float f) { return *this += Interval(f); }
    SPECULA_CPU_GPU Interval &operator-=(Float f) { return *this -= Interval(f); }
    SPECULA_CPU_GPU Interval &operator*=(Float f) {
      if (f > 0) {
        *this = Interval(mul_round_down(f, low), mul_round_up(f, high));
      } else {
        *this = Interval(mul_round_down(f, high), mul_round_up(f, low));
      }
      return *this;
    }
    SPECULA_CPU_GPU Interval &operator/=(Float f) {
      if (f > 0) {
        *this = Interval(div_round_down(low, f), div_round_up(high, f));
      } else {
        *this = Interval(div_round_down(high, f), div_round_up(low, f));
      }
      return *this;
    }

  private:
    Float low, high;

    friend struct fmt::formatter<Interval>;
  };

  SPECULA_CPU_GPU inline bool in_range(Float v, Interval i) {
    return v >= i.lower_bound() && v <= i.upper_bound();
  }
  SPECULA_CPU_GPU inline bool in_range(Interval a, Interval b) {
    return a.lower_bound() <= b.upper_bound() && a.upper_bound() >= b.lower_bound();
  }

  SPECULA_CPU_GPU inline Interval Interval::operator/(Interval i) const {
    if (in_range(0, i)) {
      return Interval(-Infinity, Infinity);
    }

    Float low_quot[4] = {div_round_down(low, i.low), div_round_down(high, i.low),
                         div_round_down(low, i.high), div_round_down(high, i.high)};
    Float high_quot[4] = {div_round_up(low, i.low), div_round_up(high, i.low),
                          div_round_up(low, i.high), div_round_up(high, i.high)};

    return {std::min({low_quot[0], low_quot[1], low_quot[2], low_quot[3]}),
            std::max({high_quot[0], high_quot[1], high_quot[2], high_quot[3]})};
  }

  SPECULA_CPU_GPU inline Interval operator+(Float f, Interval i) { return Interval(f) + i; }
  SPECULA_CPU_GPU inline Interval operator-(Float f, Interval i) { return Interval(f) - i; }
  SPECULA_CPU_GPU inline Interval operator*(Float f, Interval i) {
    if (f > 0) {
      return Interval(mul_round_down(f, i.lower_bound()), mul_round_up(f, i.upper_bound()));
    } else {
      return Interval(mul_round_down(f, i.upper_bound()), mul_round_up(f, i.lower_bound()));
    }
  }
  SPECULA_CPU_GPU inline Interval operator/(Float f, Interval i) {
    if (in_range(0, i)) {
      return Interval(-Infinity, Infinity);
    }

    if (f > 0) {
      return Interval(div_round_down(f, i.upper_bound()), div_round_up(f, i.lower_bound()));
    } else {
      return Interval(div_round_down(f, i.lower_bound()), div_round_up(f, i.upper_bound()));
    }
  }
  SPECULA_CPU_GPU inline Interval operator+(Interval i, Float f) { return i + Interval(f); }
  SPECULA_CPU_GPU inline Interval operator-(Interval i, Float f) { return i - Interval(f); }
  SPECULA_CPU_GPU inline Interval operator*(Interval i, Float f) {
    if (f > 0) {
      return Interval(mul_round_down(f, i.lower_bound()), mul_round_up(f, i.upper_bound()));
    } else {
      return Interval(mul_round_down(f, i.upper_bound()), mul_round_up(f, i.lower_bound()));
    }
  }
  SPECULA_CPU_GPU inline Interval operator/(Interval i, Float f) {
    if (in_range(0, i)) {
      return Interval(-Infinity, Infinity);
    }

    if (f > 0) {
      return Interval(div_round_down(f, i.lower_bound()), div_round_up(f, i.upper_bound()));
    } else {
      return Interval(div_round_down(f, i.upper_bound()), div_round_up(f, i.lower_bound()));
    }
  }

  SPECULA_CPU_GPU inline Interval sqr(Interval i) {
    Float alow = std::abs(i.lower_bound()), ahigh = std::abs(i.upper_bound());
    if (alow > ahigh) {
      pstd::swap(alow, ahigh);
    }
    if (in_range(0, i)) {
      return Interval(0, mul_round_up(ahigh, ahigh));
    }
    return Interval(mul_round_down(alow, alow), mul_round_up(ahigh, ahigh));
  }

  SPECULA_CPU_GPU inline Interval mul_pow2(Interval i, Float s) {
    Float as = std::abs(s);
    if (as < 1) {
      DASSERT_EQ(1 / as, 1ull << log2_int(1 / as));
    } else {
      DASSERT_EQ(as, 1ull << log2_int(as));
    }
    return Interval(std::min(i.lower_bound() * s, i.upper_bound() * s),
                    std::max(i.lower_bound() * s, i.upper_bound() * s));
  }
  SPECULA_CPU_GPU inline Interval mul_pow2(Float s, Interval i) { return mul_pow2(i, s); }

  SPECULA_CPU_GPU inline Float floor(Interval i) { return pstd::floor(i.lower_bound()); }
  SPECULA_CPU_GPU inline Float ceil(Interval i) { return pstd::ceil(i.upper_bound()); }

  SPECULA_CPU_GPU inline Float min(Interval a, Interval b) {
    return std::min(a.lower_bound(), b.lower_bound());
  }
  SPECULA_CPU_GPU inline Float max(Interval a, Interval b) {
    return std::max(a.upper_bound(), b.upper_bound());
  }

  SPECULA_CPU_GPU inline Interval sqrt(Interval i) {
    return {sqrt_round_down(i.lower_bound()), sqrt_round_up(i.upper_bound())};
  }

  SPECULA_CPU_GPU inline Interval fma(Interval a, Interval b, Interval c) {
    Float low = std::min({
        fma_round_down(a.lower_bound(), b.lower_bound(), c.lower_bound()),
        fma_round_down(a.upper_bound(), b.lower_bound(), c.lower_bound()),
        fma_round_down(a.lower_bound(), b.upper_bound(), c.lower_bound()),
        fma_round_down(a.upper_bound(), b.upper_bound(), c.lower_bound()),
    });
    Float high = std::max({
        fma_round_up(a.lower_bound(), b.lower_bound(), c.upper_bound()),
        fma_round_up(a.upper_bound(), b.lower_bound(), c.upper_bound()),
        fma_round_up(a.lower_bound(), b.upper_bound(), c.upper_bound()),
        fma_round_up(a.upper_bound(), b.upper_bound(), c.upper_bound()),
    });

    return Interval(low, high);
  }

  SPECULA_CPU_GPU inline Interval difference_of_products(Interval a, Interval b, Interval c,
                                                         Interval d) {
    Float ab[4] = {a.lower_bound() * b.lower_bound(), a.upper_bound() * b.lower_bound(),
                   a.lower_bound() * b.upper_bound(), a.upper_bound() * b.upper_bound()};
    Float ab_low = std::min({ab[0], ab[1], ab[2], ab[3]});
    Float ab_high = std::max({ab[0], ab[1], ab[2], ab[3]});

    int ab_low_index = ab_low == ab[0] ? 0 : (ab_low == ab[1] ? 1 : (ab_low == ab[2] ? 2 : 3));
    int ab_high_index = ab_high == ab[0] ? 0 : (ab_high == ab[1] ? 1 : (ab_high == ab[2] ? 2 : 3));

    Float cd[4] = {c.lower_bound() * d.lower_bound(), c.upper_bound() * d.lower_bound(),
                   c.lower_bound() * d.upper_bound(), c.upper_bound() * d.upper_bound()};
    Float cd_low = std::min({cd[0], cd[1], cd[2], cd[3]});
    Float cd_high = std::max({cd[0], cd[1], cd[2], cd[3]});

    int cd_low_index = cd_low == cd[0] ? 0 : (cd_low == cd[1] ? 1 : (cd_low == cd[2] ? 2 : 3));
    int cd_high_index = cd_high == cd[0] ? 0 : (cd_high == cd[1] ? 1 : (cd_high == cd[2] ? 2 : 3));

    Float low = difference_of_products(a[ab_low_index & 1], a[ab_low_index >> 1],
                                       c[cd_high_index & 1], d[cd_high_index >> 1]);
    Float high = difference_of_products(a[ab_high_index & 1], b[ab_high_index >> 1],
                                        c[cd_low_index & 1], d[cd_low_index >> 1]);
    DASSERT_LE(low, high);
    return {next_float_down(next_float_down(low)), next_float_up(next_float_up(high))};
  }

  SPECULA_CPU_GPU inline Interval sum_of_products(Interval a, Interval b, Interval c, Interval d) {
    return difference_of_products(a, b, -c, d);
  }

  SPECULA_CPU_GPU inline Interval abs(Interval i) {
    if (i.lower_bound() >= 0) {
      return i;
    } else if (i.upper_bound() <= 0) {
      return {-i.upper_bound(), -i.lower_bound()};
    } else {
      return {0, std::max(-i.lower_bound(), i.upper_bound())};
    }
  }

  SPECULA_CPU_GPU inline Interval acos(Interval i) {
    Float low = std::acos(std::min<Float>(1, i.upper_bound()));
    Float high = std::acos(std::max<Float>(-1, i.lower_bound()));

    return Interval(std::max<Float>(0, next_float_down(low)), next_float_up(high));
  }

  SPECULA_CPU_GPU inline Interval sin(Interval i) {
    ASSERT_GE(i.lower_bound(), -1e-16);
    ASSERT_LE(i.upper_bound(), 2.0001 * Pi);
    Float low = std::cos(std::max<Float>(0, i.lower_bound()));
    Float high = std::cos(i.upper_bound());
    if (low > high) {
      pstd::swap(low, high);
    }
    low = std::max<Float>(-1, next_float_down(low));
    high = std::min<Float>(1, next_float_up(high));
    if (in_range(Pi, i)) {
      low = -1;
    }
    return Interval(low, high);
  }

  SPECULA_CPU_GPU inline Interval cos(Interval i) {
    ASSERT_GE(i.lower_bound(), -1e-16);
    ASSERT_LE(i.upper_bound(), 2.0001 * Pi);
    Float low = std::cos(std::max<Float>(0, i.lower_bound()));
    Float high = std::cos(i.upper_bound());
    if (low > high) {
      pstd::swap(low, high);
    }
    low = std::max<Float>(-1, next_float_down(low));
    high = std::min<Float>(1, next_float_up(high));
    if (in_range(Pi, i)) {
      low = -1;
    }
    return Interval(low, high);
  }

  SPECULA_CPU_GPU inline bool quadratic(Interval a, Interval b, Interval c, Interval *t0,
                                        Interval *t1) {
    Interval discrim = difference_of_products(b, b, mul_pow2(4, a), c);
    if (discrim.lower_bound() < 0) {
      return false;
    }
    Interval float_root_discrim = sqrt(discrim);
    Interval denom = mul_pow2(2, a);
    Interval q;

    if ((Float)b < 0) {
      q = mul_pow2(-0.5, b - float_root_discrim);
    } else {
      q = mul_pow2(-0.5, b + float_root_discrim);
    }
    *t0 = q / a;
    *t1 = c / q;
    if (t0->lower_bound() > t1->lower_bound()) {
      pstd::swap(*t0, *t1);
    }
    return true;
  }

  SPECULA_CPU_GPU inline Interval sum_squares(Interval i) { return sqr(i); }

  template <typename... Args>
  SPECULA_CPU_GPU inline Interval sum_squares(Interval i, Args... args) {
    Interval ss = fma(i, i, sum_squares(args...));
    return Interval(std::max<Float>(0, ss.lower_bound()), ss.upper_bound());
  }

} // namespace specula

template <> struct fmt::formatter<specula::Interval> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  inline auto format(const specula::Interval &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ Interval [{} {}] ]", v.low, v.high);
  }
};

#endif // INCLUDE_MATH_INTERVAL_HPP_
