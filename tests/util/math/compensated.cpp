#include "specula/util/math/compensated.hpp"

#include <catch2/catch_test_macros.hpp>

#include "specula/util/math/functions.hpp"
#include "specula/util/rng.hpp"

using namespace specula;

static Float get_float(Rng &rng, Float min_exp = -10.0, Float max_exp = 10.0) {
  Float logu = lerp(rng.uniform<Float>(), min_exp, max_exp);
  Float val = std::pow(10, logu);
  Float sign = rng.uniform<Float>() < 0.5 ? -1.0 : 1.0;
  return val * sign;
}

TEST_CASE("CompensatedSum", "[util][math]") {
  CompensatedSum<double> kahan_sumd;
  long double ld_sum = 0;
  double double_sum = 0;
  CompensatedSum<float> kahan_sumf;
  float float_sum = 0;

  Rng rng;
  for (int i = 0; i < 16 * 1024 * 1024; ++i) {
    float v = std::exp(lerp(rng.uniform<Float>(), -5, 20));
    ld_sum += v;
    kahan_sumd += v;
    double_sum += v;
    kahan_sumf += v;
    float_sum += v;
  }

  int64_t kahand_bits = float_to_bits(double(kahan_sumd));
  int64_t ld_bits = float_to_bits(double(ld_sum));
  int64_t double_bits = float_to_bits(double(double_sum));
  int64_t kahanf_bits = float_to_bits(double(float(kahan_sumf)));
  int64_t float_bits = float_to_bits(double(float_sum));

  int64_t ld_error_ulps = std::abs(ld_bits - kahand_bits);
  int64_t double_error_ulps = std::abs(double_bits - kahand_bits);
  int64_t kahanf_error_ulps = std::abs(kahanf_bits - kahand_bits);
  int64_t float_error_ulps = std::abs(float_bits - kahand_bits);

  if (sizeof(long double) > sizeof(double)) {
    CHECK(ld_error_ulps * 10'000 < double_error_ulps);
  }
  CHECK(double_error_ulps * 1'000 < kahanf_error_ulps);
  CHECK(kahanf_error_ulps * 10'000 < float_error_ulps);
}

TEST_CASE("CompensatedFloat", "[util][math]") {
  SECTION("two_prod") {
    for (int i = 0; i < 100'000; ++i) {
      Rng rng(i);

      Float a = get_float(rng), b = get_float(rng);
      CompensatedFloat tp = two_prod(a, b);
      CHECK((Float)tp == a * b);
      CHECK((double)tp == (double)a * (double)b);
    }
  }

  SECTION("two_sum") {
    for (int i = 0; i < 100'000; ++i) {
      Rng rng(i);

      Float a = get_float(rng), b = get_float(rng);
      CompensatedFloat tp = two_sum(a, b);
      CHECK((Float)tp == a + b);
      CHECK((double)tp == (double)a + (double)b);
    }
  }

#ifndef SPECULA_FLOAT_AS_DOUBLE
  SECTION("inner_product") {
    for (int i = 0; i < 100000; ++i) {
      Rng rng(i);

      Float a[4] = {get_float(rng), get_float(rng), get_float(rng), get_float(rng)};
      Float b[4] = {get_float(rng), get_float(rng), get_float(rng), get_float(rng)};
      Float ab = inner_product(a[0], b[0], a[1], b[1], a[2], b[2], a[3], b[3]);
      Float dab = double(a[0]) * double(b[0]) + double(a[1]) * double(b[1]) +
                  double(a[2]) * double(b[2]) + double(a[3]) * double(b[3]);
      CHECK(ab == dab);
    }
  }

#endif
}
