#include "specula/util/math/interval.hpp"

#include <algorithm>

#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>

#include "specula/util/float.hpp"
#include "specula/util/math/functions.hpp"
#include "specula/util/rng.hpp"

using namespace specula;

static const int FLOAT_INTERVAL_ITERS = 1'000'000;

static Interval get_float(Rng &rng, Float min_exp = -6.0, Float max_exp = 6.0) {
  Float logu = lerp(rng.uniform<Float>(), min_exp, max_exp);
  Float val = std::pow(10, logu);

  Float err = 0;
  switch (rng.uniform<uint32_t>(4)) {
  case 0:
    break;
  case 1: {
    auto ulp_error = rng.uniform<uint32_t>(1024);
    Float offset = bits_to_float(float_to_bits(val) + ulp_error);
    err = std::abs(offset - val);
    break;
  }
  case 2: {
    auto ulp_error = rng.uniform<uint32_t>(1024 * 1024);
    Float offset = bits_to_float(float_to_bits(val) + ulp_error);
    err = std::abs(offset - val);
    break;
  }
  case 3: {
    err = (4 * rng.uniform<Float>()) * std::abs(val);
  }
  }

  Float sign = rng.uniform<Float>() < 0.5 ? -1.0 : 1.0;
  return Interval::from_value_and_error(sign * val, err);
}

static double get_percise(const Interval &ef, Rng &rng) {
  switch (rng.uniform<uint32_t>(3)) {
  case 0:
    return ef.lower_bound();
  case 1:
    return ef.upper_bound();
  case 2: {
    Float t = rng.uniform<Float>();
    double p = (1 - t) * ef.lower_bound() + t * ef.upper_bound();
    p = std::min<double>(p, ef.upper_bound());
    p = std::max<double>(p, ef.lower_bound());
    return p;
  }
  default:
    return (Float)ef;
  }
}

TEST_CASE("Interval", "[util][math]") {
  SECTION("abs") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval ef = get_float(rng);
      double percise = get_percise(ef, rng);

      Interval ef_result = abs(ef);
      double percise_result = std::abs(percise);

      CHECK(percise_result >= ef_result.lower_bound());
      CHECK(percise_result <= ef_result.upper_bound());
    }
  }

  SECTION("sqrt") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval ef = get_float(rng);
      double percise = get_percise(ef, rng);

      Interval ef_result = sqrt(abs(ef));
      double percise_result = std::sqrt(std::abs(percise));

      CHECK(percise_result >= ef_result.lower_bound());
      CHECK(percise_result <= ef_result.upper_bound());
    }
  }

  SECTION("addition") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval ef[2] = {get_float(rng), get_float(rng)};
      double percise[2] = {get_percise(ef[0], rng), get_percise(ef[1], rng)};

      Interval ef_result = ef[0] + ef[1];
      double percise_result = percise[0] + percise[1];

      CHECK(percise_result >= ef_result.lower_bound());
      CHECK(percise_result <= ef_result.upper_bound());
    }
  }

  SECTION("subtraction") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval ef[2] = {get_float(rng), get_float(rng)};
      double percise[2] = {get_percise(ef[0], rng), get_percise(ef[1], rng)};

      Interval ef_result = ef[0] - ef[1];
      double percise_result = percise[0] - percise[1];

      CHECK(percise_result >= ef_result.lower_bound());
      CHECK(percise_result <= ef_result.upper_bound());
    }
  }

  SECTION("multiplication") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval ef[2] = {get_float(rng), get_float(rng)};
      double percise[2] = {get_percise(ef[0], rng), get_percise(ef[1], rng)};

      Interval ef_result = ef[0] * ef[1];
      double percise_result = percise[0] * percise[1];

      CHECK(percise_result >= ef_result.lower_bound());
      CHECK(percise_result <= ef_result.upper_bound());
    }
  }

  SECTION("division") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval ef[2] = {get_float(rng), get_float(rng)};
      double percise[2] = {get_percise(ef[0], rng), get_percise(ef[1], rng)};

      Interval ef_result = ef[0] / ef[1];
      double percise_result = percise[0] / percise[1];

      CHECK(percise_result >= ef_result.lower_bound());
      CHECK(percise_result <= ef_result.upper_bound());
    }
  }

  SECTION("fma") {
    int n_trials = 10'000, n_iters = 400;
    int ratio_count = 0;
    int n_better = 0;
    for (int i = 0; i < n_trials; ++i) {
      Rng rng(i);
      Interval v = abs(get_float(rng));
      for (int j = 0; j < n_iters; ++j) {
        Interval a = v;
        Interval b = get_float(rng);
        Interval c = get_float(rng);

        v = fma(a, b, c);
        if (std::isinf(v.lower_bound()) || std::isinf(v.upper_bound())) {
          break;
        }

        double pa = get_percise(a, rng);
        double pb = get_percise(b, rng);
        double pc = get_percise(c, rng);

        double percise_result = specula::fma(pa, pb, pc);
        Interval vp = a * b + c;

        INFO(fmt::format("v = {} vp = {}", v, vp));

        CHECK(percise_result >= v.lower_bound());
        CHECK(percise_result <= v.upper_bound());
        CHECK(v.lower_bound() >= vp.lower_bound());
        CHECK(v.upper_bound() <= vp.upper_bound());

        n_better += static_cast<int>(v.lower_bound() > vp.lower_bound() ||
                                     v.upper_bound() < vp.upper_bound());
      }
    }

    CHECK(n_better > 0.85 * ratio_count);
  }

  SECTION("sqr") {
    Interval a = Interval(1.75, 2.25);
    Interval as = sqr(a), at = a * a;
    CHECK(as.upper_bound() == at.upper_bound());
    CHECK(as.lower_bound() == at.lower_bound());

    Interval b = Interval(-0.75, 1.25);
    Interval bs = sqr(b), bt = b * b;
    CHECK(bs.upper_bound() == bt.upper_bound());
    CHECK(0 == bs.lower_bound());
    CHECK(bt.lower_bound() < 0);
  }

  SECTION("sum_squares") {
    Interval a(1), b(2), c(3);
    CHECK(1 == Float(sum_squares(a)));
    CHECK(4 == Float(sum_squares(b)));
    CHECK(5 == Float(sum_squares(a, b)));
    CHECK(14 == Float(sum_squares(a, b, c)));
  }

  SECTION("difference_of_products") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval a = abs(get_float(rng));
      Interval b = abs(get_float(rng));
      Interval c = abs(get_float(rng));
      Interval d = abs(get_float(rng));

      Float sign = rng.uniform<Float>() < -0.5 ? -1 : 1;
      b *= sign;
      c *= sign;

      double pa = get_percise(a, rng);
      double pb = get_percise(b, rng);
      double pc = get_percise(c, rng);
      double pd = get_percise(d, rng);

      Interval r = difference_of_products(a, b, c, d);
      double pr = difference_of_products(pa, pb, pc, pd);

      CHECK(pr >= r.lower_bound());
      CHECK(pr <= r.upper_bound());
    }
  }

  SECTION("sum_of_products") {
    for (int trial = 0; trial < FLOAT_INTERVAL_ITERS; ++trial) {
      Rng rng(trial);

      Interval a = abs(get_float(rng));
      Interval b = abs(get_float(rng));
      Interval c = abs(get_float(rng));
      Interval d = -abs(get_float(rng));

      Float sign = rng.uniform<Float>() < -0.5 ? -1 : 1;
      b *= sign;
      c *= sign;

      double pa = get_percise(a, rng);
      double pb = get_percise(b, rng);
      double pc = get_percise(c, rng);
      double pd = get_percise(d, rng);

      Interval r = sum_of_products(a, b, c, d);
      double pr = sum_of_products(pa, pb, pc, pd);

      CHECK(pr >= r.lower_bound());
      CHECK(pr <= r.upper_bound());
    }
  }
}
