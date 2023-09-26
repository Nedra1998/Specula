#include <limits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <specula/util/float.hpp>

#include "specula/specula.hpp"

using namespace specula;

TEST_CASE("Floating point", "[util]") {
  SECTION("Float pieces") {
    CHECK(1 == exponent(2.f));
    CHECK(-1 == exponent(0.5f));
    CHECK(0b10000000000000000000000 == significand(3.f));
    CHECK(0b11000000000000000000000 == significand(7.f));
  }

  SECTION("Double pieces") {
    CHECK(1 == exponent(2.));
    CHECK(-1 == exponent(0.5));
    CHECK(0b1000000000000000000000000000000000000000000000000000 == specula::significand(3.));
    CHECK(0b1100000000000000000000000000000000000000000000000000 == specula::significand(7.));
  }

  SECTION("Float bits") {
    uint32_t ui = GENERATE(take(1000, random(0x0u, 0xffffffffu)));
    float f = bits_to_float(ui);
    if (!std::isnan(f))
      CHECK(ui == float_to_bits(f));
  }

  SECTION("Double bits") {
    uint64_t ui = GENERATE(take(1000, random(0x0ull, 0xffffffffull)));
    double f = bits_to_float(ui);
    if (!std::isnan(f))
      CHECK(ui == float_to_bits(f));
  }

  SECTION("Next float up/down") {
    CHECK(next_float_up(-0.f) > 0.f);
    CHECK(next_float_down(0.f) < 0.f);

    CHECK(next_float_up((float)Infinity) == (float)Infinity);
    CHECK(next_float_down((float)Infinity) < (float)Infinity);

    CHECK(next_float_down(-(float)Infinity) == -(float)Infinity);
    CHECK(next_float_up(-(float)Infinity) > -(float)Infinity);
  }

  SECTION("Next float up/down - Fuzzing") {
    uint32_t ui = GENERATE(take(1000, random(0x0u, 0xffffffffu)));
    float f = bits_to_float(ui);
    if (!std::isnan(f) && !std::isinf(f)) {
      CHECK(std::nextafter(f, (float)Infinity) == next_float_up(f));
      CHECK(std::nextafter(f, -(float)Infinity) == next_float_down(f));
    }
  }

  SECTION("Next double up/down") {
    CHECK(next_float_up(-0.) > 0.);
    CHECK(next_float_down(0.) < 0.);

    CHECK(next_float_up((double)Infinity) == (double)Infinity);
    CHECK(next_float_down((double)Infinity) < (double)Infinity);

    CHECK(next_float_down(-(double)Infinity) == -(double)Infinity);
    CHECK(next_float_up(-(double)Infinity) > -(double)Infinity);
  }

  SECTION("Next double up/down - Fuzzing") {
    uint64_t ui = GENERATE(take(1000, random(0x0ull, 0xffffffffull)));
    double d = bits_to_float(ui);
    if (!std::isnan(d) && !std::isinf(d)) {
      CHECK(std::nextafter(d, (double)Infinity) == next_float_up(d));
      CHECK(std::nextafter(d, -(double)Infinity) == next_float_down(d));
    }
  }
}

TEST_CASE("Half", "[util]") {
  SECTION("Basics") {
    CHECK(Half(0.f).bits() == HalfPositiveZero);
    CHECK(Half(-0.f).bits() == HalfNegativeZero);
    CHECK(Half(Infinity).bits() == HalfPositiveInfinity);
    CHECK(Half(-Infinity).bits() == HalfNegativeInfinity);

    CHECK(Half(std::numeric_limits<float>::quiet_NaN()).isnan());
    CHECK(Half(std::numeric_limits<double>::quiet_NaN()).isnan());
    CHECK_FALSE(Half::from_bits(HalfPositiveInfinity).isnan());

    for (uint32_t bits = 0; bits < 65536; ++bits) {
      Half h = Half::from_bits(bits);
      if (h.isnan() || h.isinf())
        continue;
      CHECK(h == -(-h));
      CHECK(-h == Half(-float(h)));
    }
  }

  SECTION("Exact conversions") {
    for (Float i = -2048; i <= 2048; ++i) {
      CHECK(i == Float(Half(i)));
    }

    float limit = 1024, delta = 0.5;
    for (int i = 0; i < 10; ++i) {
      for (float f = -limit; f <= limit; f += delta)
        CHECK(f == Float(Half(f)));
      limit /= 2;
      delta /= 2;
    }
  }

  SECTION("Randoms") {
    float f = GENERATE(take(1000, random(0.f, 512.f)));
    uint16_t h = Half(f).bits();
    float fh = Float(Half::from_bits(h));
    if (fh == f)
      SKIP();

    uint16_t hother;
    if (fh > f) {
      hother = h - 1;
      if (hother > h)
        SKIP();
    } else {
      hother = h + 1;
      if (hother < h)
        SKIP();
    }

    float fother = Float(Half::from_bits(hother));
    float dh = std::abs(fh - f);
    float dother = std::abs(fother - f);
    if (fh > f)
      CHECK(fother < f);
    else
      CHECK(fother > f);

    CHECK(dh < dother);
  }

  SECTION("Next up") {
    Half h = Half::from_bits(HalfNegativeInfinity);
    int iters = 0;
    while (h.bits() != HalfPositiveInfinity) {
      REQUIRE(iters < 65536);
      ++iters;

      Half hup = h.next_up();
      CHECK((float)hup > (float)h);
      h = hup;
    }
    CHECK(65536 - (1 << 11) == iters);
  }

  SECTION("Next down") {
    Half h = Half::from_bits(HalfPositiveInfinity);
    int iters = 0;
    while (h.bits() != HalfNegativeInfinity) {
      REQUIRE(iters < 65536);
      ++iters;

      Half hdown = h.next_down();
      CHECK((float)hdown < (float)h);
      h = hdown;
    }
    CHECK(65536 - (1 << 11) == iters);
  }

  SECTION("Equal") {
    for (uint32_t bits = 0; bits < 65536; ++bits) {
      Half h = Half::from_bits(bits);
      if (h.isnan() || h.isinf())
        continue;
      CHECK(h == h);
    }

    Half hpz(0.f), hnz(-0.f);
    CHECK(hpz.bits() != hnz.bits());
    CHECK(hpz == hnz);

    Half hmin(5.9605e-08f);
    CHECK(hpz != hmin);
    CHECK(hnz != -hmin);
  }

  SECTION("Round to nearest even") {
    Half h0(0.f), h1 = h0.next_up();
    while (!h1.isinf()) {
      float mid = (float(h0) + float(h1)) / 2;
      if ((h0.bits() & 1) == 0)
        CHECK(h0 == Half(mid));
      else
        CHECK(h1 == Half(mid));

      h0 = h1;
      h1 = h0.next_up();
    }

    h0 = Half(-0.f);
    h1 = h0.next_down();
    while (!h1.isinf()) {
      float mid = (float(h0) + float(h1)) / 2;
      if ((h0.bits() & 1) == 0)
        CHECK(h0 == Half(mid));
      else
        CHECK(h1 == Half(mid));

      h0 = h1;
      h1 = h0.next_up();
    }
  }
}
