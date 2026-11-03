#include "specula/util/sampling/rng_iterator.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <spdlog/common.h>

#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("RngIterators", "[util][sampling]") {
  SECTION("Uniform1D") {
    int count = 0;
    for (Float u : Uniform1D(120)) {
      CHECK(u >= 0);
      CHECK(u < 1);
      ++count;
    }
    CHECK(count == 120);
  }

  SECTION("Seeded Uniform1D") {
    std::vector<Float> samples;
    for (Float u : Uniform1D(1250)) {
      samples.push_back(u);
    }

    int i = 0;
    for (Float u : Uniform1D(samples.size(), 1)) {
      CHECK(u != samples[i]);
      ++i;
    }
  }

  SECTION("Uniform2D") {
    int count = 0;
    for (Point2f u : Uniform2D(32)) {
      CHECK(u[0] >= 0);
      CHECK(u[0] < 1);
      CHECK(u[1] >= 0);
      CHECK(u[1] < 1);
      ++count;
    }
    CHECK(count == 32);
  }

  SECTION("Seeded Uniform1D") {
    std::vector<Point2f> samples;
    for (Point2f u : Uniform2D(83)) {
      samples.push_back(u);
    }

    int i = 0;
    for (Point2f u : Uniform2D(samples.size(), 1)) {
      CHECK(u != samples[i]);
      ++i;
    }
  }

  SECTION("Uniform3D") {
    int count = 0;
    for (Point3f u : Uniform3D(32)) {
      CHECK(u[0] >= 0);
      CHECK(u[0] < 1);
      CHECK(u[1] >= 0);
      CHECK(u[1] < 1);
      CHECK(u[2] >= 0);
      CHECK(u[2] < 1);
      ++count;
    }
    CHECK(count == 32);
  }

  SECTION("Stratified1D") {
    int count = 0, n = 128;
    for (Float u : Stratified1D(n)) {
      CHECK(u >= Float(count) / Float(n));
      CHECK(u < Float(count + 1) / Float(n));
      ++count;
    }
    CHECK(count == n);
  }

  SECTION("Stratified2D") {
    int count = 0, nx = 16, ny = 4;
    for (Point2f u : Stratified2D(nx, ny)) {
      int ix = count % nx;
      int iy = count / nx;
      CHECK(u[0] >= Float(ix) / Float(nx));
      CHECK(u[0] < Float(ix + 1) / Float(nx));
      CHECK(u[1] >= Float(iy) / Float(ny));
      CHECK(u[1] < Float(iy + 1) / Float(ny));
      ++count;
    }
    CHECK(count == nx * ny);
  }

  SECTION("Stratified3D") {
    int count = 0, nx = 4, ny = 32, nz = 8;
    for (Point3f u : Stratified3D(nx, ny, nz)) {
      int ix = count % nx;
      int iy = (count / nx) % ny;
      int iz = count / (nx * ny);
      CHECK(u[0] >= Float(ix) / Float(nx));
      CHECK(u[0] < Float(ix + 1) / Float(nx));
      CHECK(u[1] >= Float(iy) / Float(ny));
      CHECK(u[1] < Float(iy + 1) / Float(ny));
      CHECK(u[2] >= Float(iz) / Float(nz));
      CHECK(u[2] < Float(iz + 1) / Float(nz));
      ++count;
    }
    CHECK(count == nx * ny * nz);
  }

  SECTION("Hammersley2D") {
    int count = 0;
    for (Point2f u : Hammersley2D(32)) {
      CHECK(u[0] == Float(count) / 32.0f);
      CHECK(u[1] == radical_inverse(0, count));
      ++count;
    }
    CHECK(count == 32);
  }

  SECTION("Hammersley3D") {
    int count = 0;
    for (Point3f u : Hammersley3D(128)) {
      CHECK(u[0] == Float(count) / 128.0f);
      CHECK(u[1] == radical_inverse(0, count));
      CHECK(u[2] == radical_inverse(1, count));
      ++count;
    }
    CHECK(count == 128);
  }
}
