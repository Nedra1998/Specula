#include "specula/util/math/functions.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/math.hpp"
#include "specula/util/rng.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Math", "[util][math]") {
  SECTION("is_power_of_2") {
    for (int i = 0; i < 32; ++i) {
      uint32_t ui = 1u << i;
      CHECK(is_power_of_2(ui));
      if (ui > 1) {
        CHECK_FALSE(is_power_of_2(ui + 1));
      }
      if (ui > 2) {
        CHECK_FALSE(is_power_of_2(ui - 1));
      }
    }
  }

  SECTION("round_up_pow_2") {
    CHECK(round_up_pow2(7) == 8);

    for (int i = 1; i < (1 << 24); ++i) {
      if (is_power_of_2(i)) {
        CHECK(round_up_pow2(i) == i);
      } else {
        CHECK(round_up_pow2(i) == 1 << (log2_int(i) + 1));
      }
    }

    for (int64_t i = 1; i < (1 << 24); ++i) {
      if (is_power_of_2(i)) {
        CHECK(round_up_pow2(i) == i);
      } else {
        CHECK(round_up_pow2(i) == 1 << (log2_int(i) + 1));
      }
    }

    for (int i = 0; i < 30; ++i) {
      int v = 1 << i;
      CHECK(round_up_pow2(v) == v);
      CHECK(round_up_pow2(v + 1) == 2 * v);
      if (v > 2) {
        CHECK(round_up_pow2(v - 1) == v);
      }
    }

    for (int i = 0; i < 62; ++i) {
      int64_t v = 1ll << i;
      CHECK(round_up_pow2(v) == v);
      CHECK(round_up_pow2(v + 1) == 2 * v);
      if (v > 2) {
        CHECK(round_up_pow2(v - 1) == v);
      }
    }
  }

  SECTION("morton2") {
    uint16_t x = 0b01010111, y = 0b11000101;
    uint32_t m = encode_morton2(x, y);
    CHECK(m == 0b1011000100110111);

    Rng rng(12351);
    for (int i = 0; i < 100'000; ++i) {
      uint32_t x = rng.uniform<uint32_t>();
      uint32_t y = rng.uniform<uint32_t>();
      uint64_t m = encode_morton2(x, y);

      uint32_t xp = 0, yp = 0;
      decode_morton2(m, &xp, &yp);
      CHECK(x == xp);
      CHECK(y == yp);
    }
  }

  SECTION("pow") {
    CHECK(pow<0>(2.f) == 1 << 0);
    CHECK(pow<1>(2.f) == 1 << 1);
    CHECK(pow<2>(2.f) == 1 << 2);
    CHECK(pow<3>(2.f) == 1 << 3);
    CHECK(pow<4>(2.f) == 1 << 4);
    CHECK(pow<5>(2.f) == 1 << 5);
    CHECK(pow<6>(2.f) == 1 << 6);
    CHECK(pow<7>(2.f) == 1 << 7);
    CHECK(pow<8>(2.f) == 1 << 8);
    CHECK(pow<9>(2.f) == 1 << 9);
    CHECK(pow<10>(2.f) == 1 << 10);
    CHECK(pow<11>(2.f) == 1 << 11);
    CHECK(pow<12>(2.f) == 1 << 12);
    CHECK(pow<13>(2.f) == 1 << 13);
    CHECK(pow<14>(2.f) == 1 << 14);
    CHECK(pow<15>(2.f) == 1 << 15);
    CHECK(pow<16>(2.f) == 1 << 16);
    CHECK(pow<17>(2.f) == 1 << 17);
    CHECK(pow<18>(2.f) == 1 << 18);
    CHECK(pow<19>(2.f) == 1 << 19);
    CHECK(pow<20>(2.f) == 1 << 20);
    CHECK(pow<21>(2.f) == 1 << 21);
    CHECK(pow<22>(2.f) == 1 << 22);
    CHECK(pow<23>(2.f) == 1 << 23);
    CHECK(pow<24>(2.f) == 1 << 24);
    CHECK(pow<25>(2.f) == 1 << 25);
    CHECK(pow<26>(2.f) == 1 << 26);
    CHECK(pow<27>(2.f) == 1 << 27);
    CHECK(pow<28>(2.f) == 1 << 28);
    CHECK(pow<29>(2.f) == 1 << 29);
  }

  SECTION("newton_bisection") {
    CHECK_THAT(
        newton_bisection(
            0, 10, [](Float x) -> std::pair<Float, Float> { return {Float(-1 + x), Float(1)}; }),
        WithinRel(Float(1)));
    CHECK_THAT(
        newton_bisection(
            0, 2, [](Float x) -> std::pair<Float, Float> { return {std::cos(x), -std::sin(x)}; }),
        WithinRel(Float(PI / 2.0)));

    CHECK_THAT(newton_bisection(0, 2,
                                [](Float x) -> std::pair<Float, Float> {
                                  return {std::cos(x), 10 * std::sin(x)};
                                }),
               WithinAbs(Float(PI / 2.0), 1e-5));

    CHECK_THAT(std::sin(newton_bisection(
                   0.1, 10.1,
                   [](Float x) -> std::pair<Float, Float> { return {std::sin(x), std::cos(x)}; })),
               WithinAbs(0, 1e-7));

    auto f = [](Float x) -> std::pair<Float, Float> {
      return {std::pow(sqr(std::sin(x)), 0.05) - 0.3,
              0.1 * std::cos(x) * std::sin(x) / std::pow(sqr(std::sin(x)), 0.95)};
    };
    CHECK_THAT(f(newton_bisection(0.01, 9.42477798, f)).first, WithinAbs(0, 1e-2));
  }

  SECTION("evaluate_polynomial") {
    CHECK(evaluate_polynomial(100, 4) == 4);
    CHECK(evaluate_polynomial(2, 4, 3) == 10);

    CHECK(evaluate_polynomial(0.5, 1.5, 2.75, -4.25, 15.125) ==
          1.5 + 2.75 * 0.5 - 4.25 * pow<2>(0.5) + 15.125 * pow<3>(0.5));
  }

  SECTION("log2_int") {
    for (int i = 0; i < 32; ++i) {
      uint32_t ui = 1u << i;
      CHECK(log2_int(ui) == i);
      CHECK(log2_int((uint64_t)ui) == i);
    }

    for (int i = 1; i < 31; ++i) {
      uint32_t ui = 1u << i;
      CHECK(log2_int(ui + 1) == i);
      CHECK(log2_int((uint64_t)(ui + 1)) == i);
    }

    for (int i = 0; i < 64; ++i) {
      uint64_t ui = 1ull << i;
      CHECK(log2_int(ui) == i);
    }

    for (int i = 1; i < 63; ++i) {
      uint64_t ui = 1ull << i;
      CHECK(log2_int(ui + 1) == i);
    }

    for (int i = 120; i >= -120; --i) {
      CHECK(log2_int(float(std::pow(2.0f, i))) == i);
    }
    for (int i = 120; i >= -120; --i) {
      CHECK(log2_int(double(std::pow(2.0, i))) == i);
    }

    for (int i = -31; i < 31; ++i) {
      float vlow = std::pow(2.0f, i + 0.499f);
      CHECK(log2_int(vlow) == i);
      float vhigh = std::pow(2.0f, i + 0.501f);
      CHECK(log2_int(vhigh) == i + 1);
    }

    for (int i = -31; i < 31; ++i) {
      double vlow = std::pow(2.0, i + 0.4999999);
      CHECK(log2_int(vlow) == i);
      double vhigh = std::pow(2.0, i + 0.5000001);
      CHECK(log2_int(vhigh) == i + 1);
    }
  }

  SECTION("log4_int") {
    CHECK(log4_int(1) == 0);
    CHECK(log4_int(4) == 1);
    CHECK(log4_int(16) == 2);
    CHECK(log4_int(64) == 3);

    int log4 = 0, next = 4;
    for (int v = 1; v < 16385; ++v) {
      if (v == next) {
        ++log4;
        next *= 4;
      }
      CHECK(log4_int(v) == log4);
    }
  }

  SECTION("pow4") {
    for (int i = 0; i < 12; ++i) {
      int p4 = 1 << (2 * i);
      CHECK(is_power_of_4(p4));
      CHECK(round_up_pow4(p4) == p4);
      CHECK(round_up_pow4(p4 + 1) == 4 * p4);
      if (i > 0) {
        CHECK(round_up_pow4(p4 - 1) == p4);
      }
    }
  }

  SECTION("next_prime") {
    CHECK(next_prime(2) == 3);
    CHECK(next_prime(10) == 11);
    CHECK(next_prime(32) == 37);
    CHECK(next_prime(37) == 37);

    auto is_prime = [](int x) {
      if ((x & 1) == 0) {
        return false;
      }

      for (int i = 3; i < std::sqrt(x) + 1; i += 3) {
        if ((x % i) == 0) {
          return false;
        }
      }
      return true;
    };

    for (int n = 3; n < 8'000; n += 3) {
      if (is_prime(n)) {
        CHECK(next_prime(n) == n);
      } else {
        CHECK(is_prime(next_prime(n)));
      }
    }
  }

  SECTION("erf_inv") {
    float xvl[] = {0., 0.1125, 0.25, .753, 1.521, 2.5115};
    for (float x : xvl) {
      Float e = std::erf(x);
      if (e < 1) {
        Float ei = erf_inv(e);
        if (x == 0) {
          CHECK(ei == 0);
        } else {
          CHECK_THAT(ei, WithinRel(x, Float(1e-4)));
        }
      }
    }
  }

#ifndef SPECULA_FLOAT_AS_DOUBLE
  SECTION("difference_of_products") {
    for (int i = 0; i < 100000; ++i) {
      Rng rng(i);
      auto r = [&rng]() {
        Float logu = lerp(rng.uniform<Float>(), -8, 8);
        return std::pow(10, logu);
      };
      Float a = r(), b = r(), c = r(), d = r();
      Float sign = rng.uniform<Float>() < -0.5 ? -1 : 1;
      b *= sign;
      c *= sign;
      Float dp = difference_of_products(a, b, c, d);
      Float dp2 = specula::fma(double(a), double(b), -double(c) * double(d));
      CHECK_THAT(dp, WithinULP(dp2, 2));
    }
  }

  SECTION("sum_of_products") {
    for (int i = 0; i < 100000; ++i) {
      Rng rng(i);
      auto r = [&rng]() {
        Float logu = lerp(rng.uniform<Float>(), -8, 8);
        return std::pow(10, logu);
      };
      Float a = r(), b = r(), c = r(), d = r();
      Float sign = rng.uniform<Float>() < -0.5 ? -1 : 1;
      b *= sign;
      c *= sign;
      Float sp = sum_of_products(a, b, c, d);
      Float sp2 = specula::fma(double(a), double(b), double(c) * double(d));
      CHECK_THAT(sp, WithinULP(sp2, 2));
    }
  }
#endif

  SECTION("fast_exp") {
    CHECK(fast_exp(0) == 1);

    Rng rng(6502);
    for (int i = 0; i < 100; ++i) {
      Float v = lerp(rng.uniform<Float>(), -20.0f, 20.0f);
      Float f = fast_exp(v);
      Float e = std::exp(v);
      CHECK_THAT(f, WithinRel(e, 0.0003f));
    }
  }

  SECTION("gaussian_integral") {
    Float mu_sigma[][2] = {{0, 1}, {0, 2}, {0, .1}, {1, 2}, {-2, 1}};
    for (auto &i : mu_sigma) {
      Rng rng;
      for (int j = 0; j < 5; ++j) {
        Float x0 = -5 + 10 * rng.uniform<Float>();
        Float x1 = -5 + 10 * rng.uniform<Float>();
        if (x0 > x1) {
          pstd::swap(x0, x1);
        }

        Float mu = i[0], sigma = i[1];
        Float sum = 0;
        int n = 8192;
        for (int k = 0; k < n; ++k) {
          Float u = (k + rng.uniform<Float>()) / n;
          Float x = lerp(u, x0, x1);
          sum += gaussian(x, mu, sigma);
        }
        Float est = (x1 - x0) * sum / n;

        Float in = gaussian_integral(x0, x1, mu, sigma);
        CHECK_THAT(est, WithinRel(in, 1e-4f) || WithinAbs(in, 1e-5f));
      }
    }
  }

  SECTION("find_interval") {
    std::vector<float> a{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    CHECK(find_interval(a.size(), [&](int index) { return a[index] <= -1; }) == 0);
    CHECK(find_interval(a.size(), [&](int index) { return a[index] <= 100; }) == a.size() - 2);

    for (size_t i = 0; i < a.size() - 1; ++i) {
      CHECK(find_interval(a.size(), [&](int index) { return a[index] <= i; }) == i);
      CHECK(find_interval(a.size(), [&](int index) { return a[index] <= i + 0.5; }) == i);

      if (i > 0) {
        CHECK(find_interval(a.size(), [&](int index) { return a[index] <= i - 0.5; }) == i - 1);
      }
    }
  }
}
