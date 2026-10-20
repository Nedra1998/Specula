#include "specula/util/float.hpp"

#include <catch2/catch_test_macros.hpp>

#include "specula/util/rng.hpp"

using namespace specula;

static float get_float(Rng &rng) {
  float f = NAN;
  do {
    f = bits_to_float(rng.uniform<uint32_t>());
  } while (std::isnan(f));
  return f;
}

static double get_double(Rng &rng) {
  double d = NAN;
  do {
    d = bits_to_float(uint64_t(rng.uniform<uint32_t>()) |
                      (uint64_t(rng.uniform<uint32_t>()) << 32));
  } while (std::isnan(d));
  return d;
}

TEST_CASE("Float", "[util]") {
  SECTION("Pieces") {
    CHECK(exponent(2.0f) == 1);
    CHECK(exponent(0.5f) == -1);
    CHECK(significand(3.0f) == 0b10000000000000000000000);
    CHECK(significand(7.0f) == 0b11000000000000000000000);

    CHECK(exponent(2.0) == 1);
    CHECK(exponent(0.5) == -1);
    // TODO: For some reason cmath defines 'significand' which conflicts with
    // mine, but it seems mostly undocumented which is annoying.
    CHECK(specula::significand(3.0) == 0b1000000000000000000000000000000000000000000000000000);
    CHECK(specula::significand(7.0) == 0b1100000000000000000000000000000000000000000000000000);
  }

  SECTION("Next float up/down") {
    CHECK(next_float_up(-0.0f) > 0.0f);
    CHECK(next_float_down(0.0f) < 0.0f);

    CHECK(next_float_up((float)INFINITY) == (float)INFINITY);
    CHECK(next_float_down((float)INFINITY) < (float)INFINITY);

    CHECK(next_float_down(-(float)INFINITY) == -(float)INFINITY);
    CHECK(next_float_up(-(float)INFINITY) > -(float)INFINITY);

    Rng rng;
    for (int i = 0; i < 100'000; ++i) {
      float f = get_float(rng);
      if (std::isinf(f)) {
        continue;
      }

      CHECK(std::nextafter(f, (float)INFINITY) == next_float_up(f));
      CHECK(std::nextafter(f, -(float)INFINITY) == next_float_down(f));
    }
  }

  SECTION("Next double up/down") {
    CHECK(next_float_up(-0.0) > 0.0);
    CHECK(next_float_down(0.0) < 0.0);

    CHECK(next_float_up((double)INFINITY) == (double)INFINITY);
    CHECK(next_float_down((double)INFINITY) < (double)INFINITY);

    CHECK(next_float_down(-(double)INFINITY) == -(double)INFINITY);
    CHECK(next_float_up(-(double)INFINITY) > -(double)INFINITY);

    Rng rng;
    for (int i = 0; i < 100'000; ++i) {
      double d = get_float(rng);
      if (std::isinf(d)) {
        continue;
      }

      REQUIRE(std::nextafter(d, (double)INFINITY) == next_float_up(d));
      REQUIRE(std::nextafter(d, -(double)INFINITY) == next_float_down(d));
    }
  }

  SECTION("Float bits") {
    Rng rng(2);
    for (int i = 0; i < 100'000; ++i) {
      uint32_t ui = rng.uniform<uint32_t>();
      float f = bits_to_float(ui);
      if (std::isnan(f)) {
        continue;
      }
      CHECK(ui == float_to_bits(f));
    }
  }

  SECTION("Double bits") {
    Rng rng(2);
    for (int i = 0; i < 100'000; ++i) {
      uint64_t ui = uint64_t(rng.uniform<uint32_t>()) | (uint64_t(rng.uniform<uint32_t>()) << 32);
      double d = bits_to_float(ui);
      if (std::isnan(d)) {
        continue;
      }
      CHECK(ui == float_to_bits(d));
    }
  }
}

TEST_CASE("Half", "[util]") {
  SECTION("Basics") {
    CHECK(Half(0.0f).bits() == HALF_POSITIVE_ZERO);
    CHECK(Half(-0.0f).bits() == HALF_NEGATIVE_ZERO);
    CHECK(Half(INFINITY).bits() == HALF_POSITIVE_INFINITY);
    CHECK(Half(-INFINITY).bits() == HALF_NEGATIVE_INFINITY);

    CHECK(Half(std::numeric_limits<Float>::quiet_NaN()).isnan());
    CHECK_FALSE(Half::from_bits(HALF_POSITIVE_INFINITY).isnan());

    for (uint32_t bits = 0; bits < 65536; ++bits) {
      Half h = Half::from_bits(bits);
      if (h.isinf() || h.isnan()) {
        continue;
      }
      CHECK(h == -(-h));
      CHECK(-h == Half(-float(h)));
    }
  }

  SECTION("Exact conversions") {
    for (Float i = -2048.0; i <= 2048.0; i += 1.0) {
      CHECK(i == Float(Half(i)));
    }

    float limit = 1024, delta = 0.5;
    for (int i = 0; i < 10; ++i) {
      for (float f = -limit; f <= limit; f += delta) {
        CHECK(f == Float(Half(f)));
      }

      limit /= 2;
      delta /= 2;
    }
  }

  SECTION("Randoms") {
    Rng rng;

    for (int i = 0; i < 1024; ++i) {
      float f = rng.uniform<Float>() * 512;
      uint16_t h = Half(f).bits();
      float fh = Float(Half::from_bits(h));
      if (fh == f) {
        continue;
      }

      uint16_t hother = 0;
      if (fh > f) {
        hother = h - 1;
        if (hother > h) {
          continue;
        }
      } else {
        hother = h + 1;
        if (hother < h) {
          continue;
        }
      }

      float fother = Float(Half::from_bits(hother));
      float dh = std::abs(fh - f);
      float dother = std::abs(fother - f);
      if (fh > f) {
        CHECK(fother < f);
      } else {
        CHECK(fother > f);
      }

      CHECK(dh <= dother);
    }
  }

  SECTION("Next up") {
    Half h = Half::from_bits(HALF_NEGATIVE_INFINITY);
    int iters = 0;
    while (h.bits() != HALF_POSITIVE_INFINITY) {
      REQUIRE(iters < 65536);
      ++iters;

      Half hup = h.next_up();
      CHECK((float)hup > (float)h);
      h = hup;
    }
    CHECK(65536 - (1 << 11) == iters);
  }

  SECTION("Next down") {
    Half h = Half::from_bits(HALF_POSITIVE_INFINITY);
    int iters = 0;
    while (h.bits() != HALF_NEGATIVE_INFINITY) {
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
      if (h.isinf() || h.isnan()) {
        continue;
      }
      CHECK(h == h);
    }

    Half hpz(0.0f), hnz(-0.0f);
    CHECK(hpz.bits() != hnz.bits());
    CHECK(hpz == hnz);

    Half hmin(5.9605e-08f);
    CHECK(hpz != hmin);
    CHECK(hnz != hmin);
  }

  SECTION("Round to nearest even") {
    Half h0(0.0f);
    Half h1 = h0.next_up();
    while (!h1.isinf()) {
      float mid = (float(h0) + float(h1)) / 2;
      if ((h0.bits() & 1) == 0) {
        CHECK(h0 == Half(mid));
      } else {
        CHECK(h1 == Half(mid));
      }

      h0 = h1;
      h1 = h0.next_up();
    }

    h0 = Half(-0.0f);
    h1 = h0.next_down();

    while (!h1.isinf()) {
      float mid = (float(h0) + float(h1)) / 2;
      if ((h0.bits() & 1) == 0) {
        CHECK(h0 == Half(mid));
      } else {
        CHECK(h1 == Half(mid));
      }

      h0 = h1;
      h1 = h0.next_down();
    }
  }
}
