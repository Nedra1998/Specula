#include "specula/util/sampling/functions.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "specula/util/float.hpp"
#include "specula/util/low_discrepancy.hpp"
#include "specula/util/math.hpp"
#include "specula/util/math/spline.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"
#include "specula/util/vecmath.hpp"

using namespace specula;
using namespace Catch::Matchers;

TEST_CASE("SampleDiscrete", "[util][sampling]") {
  SECTION("Basics") {
    Float pdf = NAN;

    CHECK(sample_discrete({Float(5)}, 0.251, &pdf) == 0);
    CHECK(pdf == 1);

    CHECK(sample_discrete({Float(0.5), Float(0.5)}, 0.0, &pdf) == 0);
    CHECK(pdf == 0.5);

    CHECK(sample_discrete({Float(0.5), Float(0.5)}, 0.499, &pdf) == 0);
    CHECK(pdf == 0.5);

    Float uremapped = NAN;
    CHECK(sample_discrete({Float(0.5), Float(0.5)}, 0.5f, &pdf, &uremapped) == 1);
    CHECK(pdf == 0.5);
    CHECK(uremapped == 0);
  }

  SECTION("vs PiecewiseConstant1D") {
    std::vector<Float> values;
    Rng rng;
    values.reserve(15);
    for (int i = 0; i < 15; ++i) {
      values.push_back(std::max<Float>(0, rng.uniform<Float>() - 0.2f));
    }

    PiecewiseConstant1D dist(values);

    for (int i = 0; i < 100; ++i) {
      Float u = rng.uniform<Float>();
      Float sdpdf = NAN;
      int sdindex = sample_discrete(values, u, &sdpdf);

      Float pcpdf = NAN;
      Float pcoffset = dist.sample(u, &pcpdf);
      int pcindex = static_cast<int>(pcoffset * values.size());
      pcpdf /= values.size();

      CHECK(sdindex == pcindex);
      CHECK_THAT(sdpdf, WithinAbs(pcpdf, 1e-3));
    }
  }
}

TEST_CASE("Sampling", "[util][sampling]") {
  SECTION("InvertUniformHemisphere") {
    for (Point2f u : Uniform2D(1000)) {
      Vector3f v = sample_uniform_hemisphere(u);
      Point2f up = invert_uniform_hemisphere_sample(v);
      CHECK_THAT(u.x, WithinAbs(up.x, 1e-3));
      CHECK_THAT(u.y, WithinAbs(up.y, 1e-3));
    }
  }

  SECTION("InvertCosineHemisphere") {
    for (Point2f u : Uniform2D(1000)) {
      Vector3f v = sample_cosine_hemisphere(u);
      Point2f up = invert_cosine_hemisphere_sample(v);
      CHECK_THAT(u.x, WithinAbs(up.x, 1e-3));
      CHECK_THAT(u.y, WithinAbs(up.y, 1e-3));
    }
  }

  SECTION("InvertUniformSphere") {
    for (Point2f u : Uniform2D(1000)) {
      Vector3f v = sample_uniform_sphere(u);
      Point2f up = invert_uniform_sphere_sample(v);
      CHECK_THAT(u.x, WithinAbs(up.x, 1e-3));
      CHECK_THAT(u.y, WithinAbs(up.y, 1e-3));
    }
  }

  SECTION("InvertUniformTriangle") {
    for (Point2f u : Uniform2D(1000)) {
      pstd::array<Float, 3> b = sample_uniform_triangle(u);
      Point2f up = invert_uniform_triangle_sample(b);
      CHECK_THAT(u.x, WithinAbs(up.x, 1e-3));
      CHECK_THAT(u.y, WithinAbs(up.y, 1e-3));
    }
  }

  SECTION("InvertUniformCone") {
    Rng rng;
    for (Point2f u : Uniform2D(1000)) {
      Float cos_theta_max = rng.uniform<Float>();
      Vector3f v = sample_uniform_cone(u, cos_theta_max);
      Point2f up = invert_uniform_cone_sample(v, cos_theta_max);
      CHECK_THAT(u.x, WithinAbs(up.x, 1e-3));
      CHECK_THAT(u.y, WithinAbs(up.y, 1e-3));
    }
  }

  SECTION("InvertUniformDiskPolar") {
    Rng rng;
    for (Point2f u : Uniform2D(1000)) {
      Point2f p = sample_uniform_disk_polar(u);
      Point2f up = invert_uniform_disk_polar_sample(p);
      CHECK_THAT(u.x, WithinAbs(up.x, 1e-3));
      CHECK_THAT(u.y, WithinAbs(up.y, 1e-3));
    }
  }

  SECTION("InvertUniformDiskConcentric") {
    Rng rng;
    for (Point2f u : Uniform2D(1000)) {
      Point2f p = sample_uniform_disk_concentric(u);
      Point2f up = invert_uniform_disk_concentric_sample(p);
      CHECK_THAT(u.x, WithinAbs(up.x, 1e-3));
      CHECK_THAT(u.y, WithinAbs(up.y, 1e-3));
    }
  }

  SECTION("Spherical triangle") {
    int count = 1024 * 1024;
    pstd::array<Point3f, 3> v = {{4, 1, 1}, {-10, 3, 3}, {-2, -8, 10}};
    Float a = 0.5 * length(cross(v[1] - v[0], v[2] - v[0]));
    Vector3f n = normalize(cross(v[1] - v[0], v[2] - v[0]));
    Point3f p(0.5, -0.4, 0.7);

    auto f = [](Point3f p) { return p.x * p.y * p.z; };

    Float sph_sum = 0, area_sum = 0;
    for (int i = 0; i < count; ++i) {
      Point2f u(radical_inverse(0, i), radical_inverse(1, i));

      Float pdf = NAN;
      pstd::array<Float, 3> bs = sample_spherical_triangle(v, p, u, &pdf);
      Point3f ptri = bs[0] * v[0] + bs[1] * v[1] + bs[2] * v[2];
      sph_sum += f(ptri) / pdf;

      pstd::array<Float, 3> ba = sample_uniform_triangle(u);
      pdf = 1 / a;
      ptri = ba[0] * v[0] + ba[1] * v[1] + (1 - ba[0] - ba[1]) * v[2];
      area_sum += f(ptri) * abs_dot(n, normalize(ptri - p)) / (pdf * distance_squared(p, ptri));
    }
    Float sph_int = sph_sum / count;
    Float area_int = area_sum / count;

    CHECK_THAT(sph_int, WithinAbs(area_int, 1e-3));
  }

  SECTION("Spherical triangle inverse") {
    Rng rng;
    auto rp = [&rng](Float low = -10, Float high = 10) {
      return Point3(specula::lerp(rng.uniform<Float>(), low, high),
                    specula::lerp(rng.uniform<Float>(), low, high),
                    specula::lerp(rng.uniform<Float>(), low, high));
    };

    for (int i = 0; i < 100; ++i) {
      pstd::array<Point3f, 3> v = {rp(), rp(), rp()};
      Point3f p = rp(-1, 1);
      if (spherical_triangle_area(normalize(v[0] - p), normalize(v[1] - p), normalize(v[2] - p)) <
          5e-3) {
        continue;
      }

      for (int j = 0; j < 100; ++j) {
        Float pdf = NAN;
        Point2f u(rng.uniform<Float>(), rng.uniform<Float>());
        pstd::array<Float, 3> bs = sample_spherical_triangle(v, p, u, &pdf);

        Point3f ptri = bs[0] * v[0] + bs[1] * v[1] + bs[2] * v[2];
        Point2f ui = invert_spherical_triangle_sample(v, p, normalize(ptri - p));

        CHECK_THAT(ui[0], WithinAbs(u[0], 0.025f));
        CHECK_THAT(ui[1], WithinAbs(u[1], 0.005));
      }
    }
  }

  SECTION("Spherical quad") {
    int count = 1024 * 1024;
    pstd::array<Point3f, 4> v = {{4, 1, 1}, {6, 1, -2}, {4, 4, 1}, {6, 4, -2}};
    Float a = length(v[0] - v[1]) * length(v[0] - v[2]);
    Vector3f n = normalize(cross(v[1] - v[0], v[2] - v[0]));
    Point3f p(0.5, -0.4, 0.7);

    auto f = [](Point3f p) { return p.x * p.y * p.z; };

    Float sph_sum = 0, area_sum = 0;
    for (Point2f u : Hammersley2D(count)) {
      Float pdf = NAN;
      Point3f pq = sample_spherical_rectangle(p, v[0], v[1] - v[0], v[2] - v[0], u, &pdf);
      sph_sum += f(pq) / pdf;

      pq = specula::lerp(u[1], specula::lerp(u[0], v[0], v[1]), specula::lerp(u[0], v[2], v[3]));
      pdf = 1 / a;
      area_sum += f(pq) * abs_dot(n, normalize(pq - p)) / (pdf * distance_squared(p, pq));
    }

    Float sph_int = sph_sum / count;
    Float area_int = area_sum / count;
    CHECK_THAT(sph_int, WithinAbs(area_int, 1e-3));
  }

  SECTION("Spherical quad inverse") {
    int count = 64 * 1024;
    Point3f v[2][2] = {{{4, 1, 1}, {6, 1, -2}}, {{4, 4, 1}, {6, 4, -2}}};

    Rng rng;
    int ntested = 0;
    for (Point2f u : Hammersley2D(count)) {
      int a = rng.uniform<int>() & 1, b = rng.uniform<int>() & 1;
      Point3f p(specula::lerp(rng.uniform<Float>(), -10, 10),
                specula::lerp(rng.uniform<Float>(), -10, 10),
                specula::lerp(rng.uniform<Float>(), -10, 10));

      Float pdf = NAN;
      Point3f pq =
          sample_spherical_rectangle(p, v[a][b], v[!a][b] - v[a][b], v[a][!b] - v[a][b], u, &pdf);
      Float solid_angle = 1 / pdf;
      if (solid_angle < 0.01) {
        continue;
      }

      ++ntested;
      Point2f ui =
          invert_spherical_rectangle_sample(p, v[a][b], v[!a][b] - v[a][b], v[a][!b] - v[a][b], pq);

      CHECK_THAT(ui[0], WithinAbs(u[0], 0.01));
      CHECK_THAT(ui[1], WithinAbs(u[1], 0.01));
    }
    CHECK(ntested > count / 2);
  }

  SECTION("Smooth step") {
    Float start = sample_smooth_step(0, 10, 20);
    CHECK_THAT(start, WithinAbs(10, 0.1));

    Float end = sample_smooth_step(1.0, -10, -5);
    CHECK_THAT(end, WithinAbs(-5, 1e-5));

    Float mid = sample_smooth_step(0.5, 0, 1);
    CHECK_THAT(mid, WithinAbs(0.733615, 1e-5));

    for (Float u : Uniform1D(1000)) {
      Float x = sample_smooth_step(u, -3, 5);
      Float ratio = smooth_step(x, -3, 5) / smooth_step_pdf(x, -3, 5);
      CHECK_THAT(ratio, WithinAbs(4, 1e-5));

      CHECK_THAT(invert_smooth_step_sample(x, -3, 5), WithinRel(u, Float(1e-2)));
    }

    auto ss = [](Float v) { return smooth_step(v, 0, 1); };
    auto values = sample_1d_function(ss, 1024, 64 * 1024, 0.0f, 1.0f);
    PiecewiseConstant1D distrib(values);
    for (Float u : Uniform1D(100, 62351)) {
      Float cx = sample_smooth_step(u, 0, 1);
      Float cp = smooth_step_pdf(cx, 0, 1);

      Float dp = NAN;
      Float dx = distrib.sample(u, &dp);
      CHECK_THAT(dx, WithinAbs(cx, 3e-3));
      CHECK_THAT(dp, WithinAbs(cp, 3e-3));
    }
  }

  SECTION("Linear") {
    int nbuckets = 32;
    std::vector<int> buckets(nbuckets, 0);
    int ranges[][2] = {{0, 1}, {1, 2}, {5, 50}, {100, 0}, {75, 50}};
    for (auto *const r : ranges) {
      Float f0 = r[0], f1 = r[1];
      int nsamples = 1000000;
      for (int i = 0; i < nsamples; ++i) {
        Float u = (i + 0.5) / nsamples;
        Float t = sample_linear(u, f0, f1);
        ++buckets[std::min<int>(t * nbuckets, nbuckets - 1)];
      }

      for (int i = 0; i < nbuckets; ++i) {
        int expected = specula::lerp(static_cast<Float>(i) / (nbuckets - 1), buckets[0],
                                     buckets[nbuckets - 1]);
        CHECK_THAT(buckets[i], WithinRel(expected, 0.01));
      }
    }

    auto lin = [](Float v) { return 1 + 3 * v; };
    auto values = sample_1d_function(lin, 1024, 64 * 1024, 0.0f, 1.0f);
    PiecewiseConstant1D distrib(values);
    for (Float u : Uniform1D(100)) {
      Float cx = sample_linear(u, 1, 4);
      Float cp = linear_pdf(cx, 1, 4);

      Float dp = NAN;
      Float dx = distrib.sample(u, &dp);
      CHECK_THAT(dx, WithinAbs(cx, 3e-3));
      CHECK_THAT(dp, WithinAbs(cp, 3e-3));
    }

    Rng rng;
    for (Float u : Uniform1D(100)) {
      Float low = rng.uniform<Float>() * 10;
      Float high = rng.uniform<Float>() * 10;
      if (low < high) {
        pstd::swap(low, high);
      }
      Float x = sample_linear(u, low, high);

      CHECK_THAT(invert_linear_sample(x, low, high), WithinAbs(u, 1e-2));
    }
  }

  SECTION("Tent") {
    CHECK_THAT(sample_tent(0.501, 1), WithinAbs(sample_tent(0.499, 1), 0.01));

    Float rad[] = {1, 2.5, 0.125};
    Rng rng;
    for (Float radius : rad) {
      auto tent = [&](Float x) { return std::max<Float>(0, 1 - std::abs(x) / radius); };

      auto values = sample_1d_function(tent, 8192, 64, -radius, radius);
      PiecewiseConstant1D distrib(values, -radius, radius);
      for (Float u : Uniform1D(100)) {
        Float tx = sample_tent(u, radius);
        Float tp = tent_pdf(tx, radius);

        Float dp = NAN;
        Float dx = distrib.sample(u, &dp);
        CHECK_THAT(dx, WithinAbs(tx, 3e-3));
        CHECK_THAT(dp, WithinAbs(tp, 3e-3));

        CHECK_THAT(invert_tent_sample(tx, radius), WithinAbs(u, 1e-2));
      }
    }
  }

  SECTION("Catmull rom") {
    std::vector<Float> nodes = {0, 0.1, 0.4, 0.88, 1.0};
    std::vector<Float> values = {0, 5, 2, 10, 5};
    std::vector<Float> cdf(values.size());

    integrate_catmull_rom(nodes, values, pstd::span<Float>(cdf));

    auto cr = [&](Float v) { return catmull_rom(nodes, values, v); };
    PiecewiseConstant1D distrib(sample_1d_function(cr, 8192, 1024, 0.0f, 1.0f));
    for (Float u : Uniform1D(100)) {
      Float cp = NAN;
      Float cx = sample_catmull_rom(nodes, values, cdf, u, nullptr, &cp);

      Float dp = NAN;
      Float dx = distrib.sample(u, &dp);
      CHECK_THAT(dx, WithinAbs(cx, 3e-3));
      CHECK_THAT(dp, WithinAbs(cp, 3e-3));
    }
  }

  SECTION("Bilinear") {
    Rng rng;
    Float quads[][4] = {{0, 0.5, 1.3, 4.7}, {1, 1, 1, 1}, {11, 0.25, 1, 20}};

    for (auto *const v : quads) {
      auto bilerp = [&](Float x, Float y) {
        return ((1 - x) * (1 - y) * v[0] + x * (1 - y) * v[1] + (1 - x) * y * v[2] + x * y * v[3]);
      };

      auto values = sample_2d_function(bilerp, 1024, 1024, 16, Bounds2f({0, 0}, {1, 1}));
      PiecewiseConstant2D distrib(values, 1024, 1024);
      for (Point2f u : Uniform2D(100)) {
        Point2f pb = sample_bilinear(u, {v, 4});
        Float bp = bilinear_pdf(pb, {v, 4});

        Float dp = NAN;
        Point2f pd = distrib.sample(u, &dp);
        CHECK_THAT(pb.x, WithinAbs(pd.x, 3e-3));
        CHECK_THAT(pb.y, WithinAbs(pd.y, 3e-3));
        CHECK_THAT(bp, WithinAbs(dp, 3e-3));

        Point2f up = invert_bilinear_sample(pb, {v, 4});
        CHECK_THAT(up.x, WithinAbs(u.x, 3e-3));
        CHECK_THAT(up.y, WithinAbs(u.y, 3e-3));
      }
    }
  }

  SECTION("Logistic") {
    Float params[][3] = {{1.0, -PI, PI}, {5, 0, 3}, {0.25, -5, -3}};

    for (auto *const p : params) {
      Float s = p[0], a = p[1], b = p[2];
      auto logistic = [&](Float v) { return trimmed_logistic(v, s, a, b); };

      auto values = sample_1d_function(logistic, 8192, 16, a, b);
      PiecewiseConstant1D distrib(values, a, b);
      for (Float u : Uniform1D(100)) {
        Float cx = sample_trimmed_logistic(u, s, a, b);
        Float cp = trimmed_logistic_pdf(cx, s, a, b);

        Float dp = NAN;
        Float dx = distrib.sample(u, &dp);
        CHECK_THAT(cx, WithinAbs(dx, 3e-3));
        CHECK_THAT(cp, WithinAbs(dp, 3e-3));

        CHECK_THAT(invert_trimmed_logistic_sample(cx, s, a, b), WithinAbs(u, 1e-2));
      }
    }
  }

  SECTION("Trimmed exponential") {
    Float params[][2] = {{1.0, 2.0}, {5, 10}, {0.25, 20}};

    for (auto *const p : params) {
      Float c = p[0], xmax = p[1];
      auto exp = [&](Float v) { return std::exp(-c * v); };

      auto values = sample_1d_function(exp, 32768, 16, 0, xmax);
      PiecewiseConstant1D distrib(values, 0, xmax);
      for (Float u : Uniform1D(100)) {
        Float cx = sample_trimmed_exponential(u, c, xmax);
        Float cp = trimmed_exponential_pdf(cx, c, xmax);

        Float dp = NAN;
        Float dx = distrib.sample(u, &dp);
        CHECK_THAT(cx, WithinAbs(dx, 1e-2));
        CHECK_THAT(cp, WithinAbs(dp, 1e-2));

        CHECK_THAT(invert_trimmed_exponential_sample(cx, c, xmax), WithinAbs(u, 1e-2));
      }
    }
  }

  SECTION("Normal") {
    Float params[][2] = {{0.0, 1.0}, {-0.5, 0.8}, {0.25, 0.005}, {3.6, 1.6}};

    for (auto *const p : params) {
      Float mu = p[0], sigma = p[1];
      auto normal = [&](Float x) {
        return 1 / std::sqrt(2 * PI * sigma * sigma) * std::exp(-sqr(x - mu) / (2 * sigma * sigma));
      };

      auto values = sample_1d_function(normal, 8192, 16, mu - 7 * sigma, mu + 7 * sigma);
      PiecewiseConstant1D distrib(values, mu - 7 * sigma, mu + 7 * sigma);
      for (Float u : Uniform1D(100)) {
        Float cx = sample_normal(u, mu, sigma);
        Float cp = normal_pdf(cx, mu, sigma);

        Float dp = NAN;
        Float dx = distrib.sample(u, &dp);
        CHECK_THAT(cx, WithinAbs(dx, 3e-3));
        CHECK_THAT(cp, WithinRel(dp, Float(0.025)));

        CHECK_THAT(invert_normal_sample(cx, mu, sigma), WithinAbs(u, 1e-2));
      }
    }
  }

  SECTION("HenyeyGreenstein Extremes") {

    for (Float g : {-0.999f, 0.999f, -1.0f, 1.0f}) {
      for (Float u : {static_cast<Float>(0), static_cast<Float>(ONE_MINUS_EPSILON)}) {
        Float pdf = NAN;
        Vector3f w = sample_henyey_greenstein(Vector3f(0, 0, -1), g, Point2f(u, 0.5), &pdf);
        CHECK_FALSE(w.has_nan());
      }
    }
  }
}
