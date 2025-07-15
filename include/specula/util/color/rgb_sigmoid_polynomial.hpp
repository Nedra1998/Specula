#ifndef INCLUDE_COLOR_RGB_SIGMOID_POLYNOMIAL_HPP_
#define INCLUDE_COLOR_RGB_SIGMOID_POLYNOMIAL_HPP_

#include "specula/specula.hpp"
#include "specula/util/float.hpp"
#include "specula/util/math.hpp"

namespace specula {
  class RgbSigmoidPolynomial {
  public:
    RgbSigmoidPolynomial() = default;

    SPECULA_CPU_GPU RgbSigmoidPolynomial(Float c0, Float c1, Float c2) : c0(c0), c1(c1), c2(c2) {}

    SPECULA_CPU_GPU Float operator()(Float lambda) const {
      return s(evaluate_polynomial(lambda, c2, c1, c0));
    }

    SPECULA_CPU_GPU Float max_value() const {
      Float result = std::max((*this)(360), (*this)(830));
      Float lambda = -c1 / (2 * c2);
      if (lambda >= 360 && lambda <= 830) {
        result = std::max(result, (*this)(lambda));
      }
      return result;
    }

  private:
    SPECULA_CPU_GPU static Float s(Float x) {
      if (isinf(x)) {
        return x > 0 ? 1 : 0;
      }
      return 0.5f + x / (2 * std::sqrt(1 + sqr(x)));
    }

    Float c0, c1, c2;

    friend struct fmt::formatter<RgbSigmoidPolynomial>;
  };
} // namespace specula

template <> struct fmt::formatter<specula::RgbSigmoidPolynomial> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  inline auto format(const specula::RgbSigmoidPolynomial &v, FormatContext &ctx) const {
    return format_to(ctx.out(), "[ RgbSigmoidPolynomial c0={} c1={} c2={} ]", v.c0, v.c1, v.c2);
  }
};

#endif // INCLUDE_COLOR_RGB_SIGMOID_POLYNOMIAL_HPP_
