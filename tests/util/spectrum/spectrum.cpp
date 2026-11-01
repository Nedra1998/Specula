#include "specula/util/spectrum.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/colorspace.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("Spectrum", "[util][spectrum]") {
  SECTION("Max value") {
    CHECK(ConstantSpectrum(2.5).max_value() == 2.5);

    CHECK(PiecewiseLinearSpectrum({Float(300), Float(380), Float(510), Float(662), Float(700)},
                                  {Float(1.5), Float(2.6), Float(10.1), Float(5.3), Float(7.7)})
              .max_value() == Float(10.1));

    CHECK_THAT(BlackbodySpectrum(5000).max_value(), WithinAbs(1.0, 0.0001));

    BlackbodySpectrum bb(5000);
    CHECK_THAT(DenselySampledSpectrum(&bb).max_value(), WithinAbs(1.0, 0.0001));

    Rng rng;
    for (int i = 0; i < 20; ++i) {
      Rgb rgb(rng.uniform<Float>(), rng.uniform<Float>(), rng.uniform<Float>());

      RgbAlbedoSpectrum sr(*RgbColorSpace::SRGB, rgb);
      Float m = sr.max_value() * 1.00001f;
      for (Float lambda = LAMBDA_MIN; lambda < LAMBDA_MAX; lambda += 0.92) {
        CHECK(sr(lambda) <= m);
      }

      RgbUnboundedSpectrum su(*RgbColorSpace::SRGB, 10 * rgb);
      m = su.max_value() * 1.00001f;
      for (Float lambda = LAMBDA_MIN; lambda < LAMBDA_MAX; lambda += 0.92) {
        CHECK(su(lambda) <= m);
      }

      RgbIlluminantSpectrum si(*RgbColorSpace::SRGB, rgb);
      m = si.max_value() * 1.00001f;
      for (Float lambda = LAMBDA_MIN; lambda < LAMBDA_MAX; lambda += 0.92) {
        CHECK(si(lambda) <= m);
      }
    }
  }

  SECTION("Sampling PDF Y") {
    Float ysum = 0;
    int n = 1000;
    for (Float u : Stratified1D(n)) {
      Float lambda = sample_visible_wavelengths(u);
      Float pdf = visible_wavelengths_pdf(lambda);
      if (pdf > 0) {
        ysum += spectra::Y()(lambda) / pdf;
      }
    }
    Float yint = ysum / n;

    CHECK_THAT(yint, WithinRel(CIE_Y_INTEGRAL, Float(1e-3)));
  }

  SECTION("Sampling PDF XYZ") {
    Float imp_sum = 0, unif_sum = 0;
    int n = 10000;
    for (Float u : Stratified1D(n)) {
      {
        Float lambda = lerp(u, LAMBDA_MIN, LAMBDA_MAX);
        Float pdf = 1.0 / (LAMBDA_MAX - LAMBDA_MIN);
        unif_sum += (spectra::X()(lambda) + spectra::Y()(lambda) + spectra::Z()(lambda)) / pdf;
      }

      Float lambda = sample_visible_wavelengths(u);
      Float pdf = visible_wavelengths_pdf(lambda);
      if (pdf > 0) {
        imp_sum += (spectra::X()(lambda) + spectra::Y()(lambda) + spectra::Z()(lambda)) / pdf;
      }
    }

    Float imp_int = imp_sum / n, unif_int = unif_sum / n;
    CHECK_THAT(imp_int, WithinRel(unif_int, Float(1e-3)));
  }
}
