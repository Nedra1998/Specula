#include "specula/util/image.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "specula/util/color.hpp"
#include "specula/util/image/image.hpp"
#include "specula/util/image/image_channel_desc.hpp"
#include "specula/util/image/image_channel_values.hpp"
#include "specula/util/image/pixel_format.hpp"
#include "specula/util/math.hpp"
#include "specula/util/rng.hpp"
#include "specula/util/sampling.hpp"

using namespace specula;
using namespace Catch::Matchers;

static pstd::vector<float> get_float_pixels(Point2i res, int nc) {
  pstd::vector<float> p;
  for (int y = 0; y < res.y; ++y) {
    for (int x = 0; x < res.x; ++x) {
      for (int c = 0; c < nc; ++c) {
        if (c == 0) {
          p.push_back(std::sin(x / 4.0) * std::cos(y / 8.0));
        } else {
          p.push_back(-0.25 + 2.0 * (c + 3 * x + 3 * y * res.x) / (res.x * res.y));
        }
      }
    }
  }
  return p;
}

static pstd::vector<uint8_t> get_u8_pixels(Point2i res, int nc) {
  pstd::vector<uint8_t> p;
  for (int y = 0; y < res.y; ++y) {
    for (int x = 0; x < res.x; ++x) {
      for (int c = 0; c < nc; ++c) {
        if (c == 0) {
          p.push_back(x + res.x * y);
        } else {
          p.push_back(x * y + c * (x - y));
        }
      }
    }
  }
  return p;
}

static void test_round_trip(const std::filesystem::path &filename) {
  Point2i res(16, 29);
  Image image(PixelFormat::FLOAT, res, {"R", "G", "B"});
  for (int y = 0; y < res.y; ++y) {
    for (int x = 0; x < res.x; ++x) {
      image.set_channels({x, y}, {
                                     static_cast<Float>(x) / static_cast<Float>(res.x - 1),
                                     static_cast<Float>(y) / static_cast<Float>(res.y - 1),
                                     static_cast<Float>(-1.5),
                                 });
    }
  }

  REQUIRE(image.write(filename));
  ImageAndMetadata read = Image::read(filename);
  REQUIRE(read.image.resolution() == res);

  ImageChannelDesc rgb_desc = read.image.get_channel_desc({"R", "G", "B"});
  REQUIRE((bool)rgb_desc);

  for (int y = 0; y < res.y; ++y) {
    for (int x = 0; x < res.x; ++x) {
      ImageChannelValues rgb = read.image.get_channels({x, y}, rgb_desc);

      for (int c = 0; c < 3; ++c) {
        if (filename.extension() == ".pfm") {
          CHECK(rgb[c] == image.get_channel({x, y}, c));
        } else if (filename.extension() == ".exr") {
          if (c == 2) {
            CHECK(rgb[c] == image.get_channel({x, y}, c));
          } else {
            CHECK_THAT(rgb[c], WithinAbs(image.get_channel({x, y}, c), 0.001));
          }
        } else {
          if (c == 2) {
            CHECK(rgb[c] == 0);
          } else {
            CHECK_THAT(rgb[c], WithinAbs(image.get_channel({x, y}, c), 0.02));
          }
        }
      }
    }
  }

  remove(filename.c_str());
}

static Float model_quantization(Float value, PixelFormat format) {
  switch (format) {
  case PixelFormat::U256:
    return clamp((value * 255.0f) + 0.5, 0, 255) * (1.0f / 255.0f);
  case PixelFormat::HALF:
    return Float(Half(value));
  case PixelFormat::FLOAT:
    return value;
  }
}

TEST_CASE("Image", "[util][image]") {
  SECTION("Basics") {
    const ColorEncoding encoding = ColorEncoding::LINEAR;
    Image y8(PixelFormat::U256, {4, 8}, {"Y"}, encoding);
    CHECK(y8.n_channels() == 1);
    CHECK(y8.bytes_used() == y8.resolution().x * y8.resolution().y);

    Image y16(PixelFormat::HALF, {4, 8}, {"Y"});
    CHECK(y16.n_channels() == 1);
    CHECK(y16.bytes_used() == 2 * y16.resolution().x * y16.resolution().y);

    Image y32(PixelFormat::FLOAT, {4, 8}, {"Y"});
    CHECK(y32.n_channels() == 1);
    CHECK(y32.bytes_used() == 4 * y32.resolution().x * y32.resolution().y);

    Image rgb8(PixelFormat::U256, {4, 8}, {"R", "G", "B"}, encoding);
    CHECK(rgb8.n_channels() == 3);
    CHECK(rgb8.bytes_used() == 3 * rgb8.resolution().x * rgb8.resolution().y);

    Image rgb16(PixelFormat::HALF, {4, 16}, {"R", "G", "B"});
    CHECK(rgb16.n_channels() == 3);
    CHECK(rgb16.bytes_used() == 2 * 3 * rgb16.resolution().x * rgb16.resolution().y);

    Image rgb32(PixelFormat::FLOAT, {4, 32}, {"R", "G", "B"});
    CHECK(rgb32.n_channels() == 3);
    CHECK(rgb32.bytes_used() == 4 * 3 * rgb32.resolution().x * rgb32.resolution().y);
  }

  SECTION("Get / set Y") {
    Point2i res(9, 3);
    pstd::vector<float> ypixels = get_float_pixels(res, 1);

    for (auto format : {PixelFormat::U256, PixelFormat::HALF, PixelFormat::FLOAT}) {
      Image image(format, res, {"Y"}, ColorEncoding::LINEAR);
      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          image.set_channel({x, y}, 0, ypixels[y * res.x + x]);
        }
      }

      ImageChannelDesc ydesc = image.get_channel_desc({"Y"});
      CHECK(bool(ydesc));
      CHECK_FALSE(bool(image.get_channel_desc({"Y0L0"})));

      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          Float v = image.get_channel({x, y}, 0);
          ImageChannelValues cv = image.get_channels({x, y});
          CHECK(cv.size() == 1);
          CHECK(cv[0] == v);

          cv = image.get_channels({x, y}, ydesc);
          CHECK(cv.size() == 1);
          CHECK(cv[0] == v);

          if (format == PixelFormat::U256) {
            CHECK_THAT(v, WithinAbs(clamp(ypixels[y * res.x + x], 0, 1), Float(0.501f / 255.0f)));
          } else {
            CHECK(v == model_quantization(ypixels[y * res.x + x], format));
          }
        }
      }
    }
  }

  SECTION("Get / Set RGB") {
    Point2i res(7, 32);
    pstd::vector<float> pixels = get_float_pixels(res, 3);

    for (auto format : {PixelFormat::U256, PixelFormat::HALF, PixelFormat::FLOAT}) {
      Image image(format, res, {"R", "G", "B"}, ColorEncoding::LINEAR);
      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          for (int c = 0; c < 3; ++c) {
            image.set_channel({x, y}, c, pixels[3 * y * res.x + 3 * x + c]);
          }
        }
      }

      ImageChannelDesc desc = image.get_channel_desc({"R", "G", "B"});
      CHECK(bool(desc));
      CHECK_FALSE(bool(image.get_channel_desc({"R", "Gxxx", "B"})));

      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          ImageChannelValues rgb = image.get_channels({x, y});
          CHECK(rgb.size() == 3);

          ImageChannelValues rgb2 = image.get_channels({x, y}, desc);
          CHECK(rgb2.size() == 3);
          CHECK(rgb2[0] == rgb[0]);
          CHECK(rgb2[1] == rgb[1]);
          CHECK(rgb2[2] == rgb[2]);

          for (int c = 0; c < 3; ++c) {
            CHECK(image.get_channel({x, y}, c) == rgb[c]);
            int offset = 3 * y * res.x + 3 * x + c;
            if (format == PixelFormat::U256) {
              CHECK_THAT(rgb[c], WithinAbs(clamp(pixels[offset], 0, 1), Float(0.501f / 255.0f)));
            } else {
              CHECK(rgb[c] == model_quantization(pixels[offset], format));
            }
          }
        }
      }
    }
  }

  SECTION("Get / Set BGR") {
    Point2i res(7, 32);
    pstd::vector<float> pixels = get_float_pixels(res, 3);

    for (auto format : {PixelFormat::U256, PixelFormat::HALF, PixelFormat::FLOAT}) {
      Image image(format, res, {"R", "G", "B"}, ColorEncoding::LINEAR);
      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          for (int c = 0; c < 3; ++c) {
            image.set_channel({x, y}, c, pixels[3 * y * res.x + 3 * x + c]);
          }
        }
      }

      ImageChannelDesc desc = image.get_channel_desc({"B", "G", "R"});
      CHECK(bool(desc));

      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          ImageChannelValues rgb = image.get_channels({x, y});
          CHECK(rgb.size() == 3);

          ImageChannelValues bgr = image.get_channels({x, y}, desc);
          CHECK(bgr.size() == 3);
          CHECK(bgr[0] == rgb[2]);
          CHECK(bgr[1] == rgb[1]);
          CHECK(bgr[2] == rgb[0]);

          for (int c = 0; c < 3; ++c) {
            CHECK(image.get_channel({x, y}, c) == rgb[c]);
            int offset = 3 * y * res.x + 3 * x + c;
            if (format == PixelFormat::U256) {
              CHECK_THAT(rgb[c], WithinAbs(clamp(pixels[offset], 0, 1), Float(0.501f / 255.0f)));
            } else {
              CHECK(rgb[c] == model_quantization(pixels[offset], format));
            }
          }
        }
      }
    }
  }

  SECTION("Copy rect out") {
    Point2i res(29, 14);
    for (auto format : {PixelFormat::U256, PixelFormat::HALF, PixelFormat::FLOAT}) {
      for (int nc = 1; nc < 4; ++nc) {
        pstd::vector<float> orig = get_float_pixels(res, nc);

        std::vector<std::string> channel_names = {"A"};
        for (int i = 1; i < nc; ++i) {
          channel_names.emplace_back(1, 'A' + 1);
        }

        Image image(format, res, channel_names, ColorEncoding::LINEAR);

        auto *orig_iter = orig.begin();
        for (int y = 0; y < res.y; ++y) {
          for (int x = 0; x < res.x; ++x) {
            for (int c = 0; c < nc; ++c, ++orig_iter) {
              image.set_channel({x, y}, c, *orig_iter);
            }
          }
        }

        Bounds2i extent({2, 3}, {5, 10});
        std::vector<float> buf(static_cast<size_t>(extent.area() * nc));
        image.copy_rect_out(extent, pstd::make_span(buf));

        auto buf_iter = buf.begin();
        for (auto piter = begin(extent); piter != end(extent); ++piter) {
          for (int c = 0; c < nc; ++c) {
            REQUIRE(buf_iter != buf.end());
            CHECK(*buf_iter == image.get_channel(*piter, c));
            ++buf_iter;
          }
        }
      }
    }
  }

  SECTION("Copy rect in") {
    Point2i res(17, 32);
    Rng rng;

    for (auto format : {PixelFormat::U256, PixelFormat::HALF, PixelFormat::FLOAT}) {
      for (int nc = 1; nc < 4; ++nc) {
        pstd::vector<float> orig = get_float_pixels(res, nc);

        std::vector<std::string> channel_names = {"A"};
        for (int i = 1; i < nc; ++i) {
          channel_names.emplace_back(1, 'A' + 1);
        }

        Image image(format, res, channel_names, ColorEncoding::LINEAR);
        auto *origIter = orig.begin();
        for (int y = 0; y < res[1]; ++y) {
          for (int x = 0; x < res[0]; ++x) {
            for (int c = 0; c < nc; ++c, ++origIter) {
              image.set_channel({x, y}, c, *origIter);
            }
          }
        }

        Bounds2i extent(Point2i(10, 23), Point2i(17, 28));
        std::vector<float> buf(static_cast<size_t>(extent.area() * nc));
        std::ranges::generate(buf, [&rng]() { return rng.uniform<Float>(); });

        image.copy_rect_in(extent, buf);

        auto buf_iter = buf.begin();
        for (auto piter = begin(extent); piter != end(extent); ++piter) {
          for (int c = 0; c < nc; ++c) {
            REQUIRE(buf_iter != buf.end());

            if (format == PixelFormat::U256) {
              CHECK_THAT(image.get_channel(*piter, c),
                         WithinAbs(clamp(*buf_iter, 0, 1), Float(0.501f / 255.0f)));
            } else {
              CHECK(image.get_channel(*piter, c) == model_quantization(*buf_iter, format));
            }
            ++buf_iter;
          }
        }
      }
    }
  }

  SECTION("Sample simple") {
    pstd::vector<float> texels = {0, 1, 0, 0};
    Image zero_one(texels, {2, 2}, {"Y"});
    PiecewiseConstant2D distrib(zero_one.get_sampling_distribution());
    Rng rng;
    for (int i = 0; i < 1000; ++i) {
      Point2f u(rng.uniform<Float>(), rng.uniform<Float>());
      Float pdf = NAN;
      Point2f p = distrib.sample(u, &pdf);

      CHECK(p.x >= 0.5);
      CHECK(p.y <= 0.5);
    }
  }

  SECTION("Sample linear") {
    int w = 500, h = 500;
    pstd::vector<float> v;
    for (int y = 0; y < h; ++y) {
      Float fy = (y + 0.5) / h;
      for (int x = 0; x < w; ++x) {
        Float fx = (x + 0.5) / w;
        v.push_back(fx + fy);
      }
    }

    Image image(v, {w, h}, {"Y"});
    PiecewiseConstant2D distrib(image.get_sampling_distribution());
    Rng rng;
    for (int i = 0; i < 1000; ++i) {
      Point2f u(rng.uniform<Float>(), rng.uniform<Float>());
      Float pdf = NAN;
      Point2f p = distrib.sample(u, &pdf);
      Float f = p.x + p.y;

      CHECK_THAT(f, WithinAbs(pdf, 3e-3));
    }
  }

  SECTION("Sample sin / cos") {
    int w = 500, h = 500;
    auto f = [](Point2f p) { return std::abs(std::sin(3.0 * p.x) * sqr(std::cos(4.0 * p.y))); };
    Float integral = 1.0 / 24.0 * sqr(std::sin(1.5)) * (8 + std::sin(8.0));
    pstd::vector<float> v;
    for (int y = 0; y < h; ++y) {
      Float fy = (y + 0.5) / h;
      for (int x = 0; x < w; ++x) {
        Float fx = (x + 0.5) / w;
        v.push_back(f({fx, fy}));
      }
    }

    Image image(v, {w, h}, {"Y"});
    PiecewiseConstant2D distrib(image.get_sampling_distribution());
    Rng rng;
    for (int i = 0; i < 1000; ++i) {
      Point2f u(rng.uniform<Float>(), rng.uniform<Float>());
      Float pdf = NAN;
      Point2f p = distrib.sample(u, &pdf);
      Float fp = f(p);

      CHECK_THAT(pdf * integral, WithinAbs(fp, Float(0.02)));
    }
  }

  SECTION("Wrap2D") {
    pstd::vector<float> texels = {0, 1, 0, 0, 0, 0, 0, 0, 0};
    Image zero_one(texels, {3, 3}, {"Y"});

    CHECK(zero_one.get_channel({1, -1}, 0, {WrapMode::CLAMP, WrapMode::CLAMP}) == 1);
    CHECK(zero_one.get_channel({1, -1}, 0, {WrapMode::BLACK, WrapMode::CLAMP}) == 1);
    CHECK(zero_one.get_channel({1, -1}, 0, {WrapMode::BLACK, WrapMode::REPEAT}) == 0);
    CHECK(zero_one.get_channel({1, -1}, 0, {WrapMode::CLAMP, WrapMode::BLACK}) == 0);

    CHECK(zero_one.get_channel({1, 3}, 0, {WrapMode::CLAMP, WrapMode::CLAMP}) == 0);
    CHECK(zero_one.get_channel({1, 3}, 0, {WrapMode::REPEAT, WrapMode::CLAMP}) == 0);
    CHECK(zero_one.get_channel({1, 3}, 0, {WrapMode::BLACK, WrapMode::REPEAT}) == 1);
    CHECK(zero_one.get_channel({1, 3}, 0, {WrapMode::CLAMP, WrapMode::BLACK}) == 0);

    CHECK(zero_one.bilerp_channel({0.5, 0.0}, 0, WrapMode::REPEAT) == 0.5);
    CHECK(zero_one.bilerp_channel({0.5, 0.0}, 0, WrapMode::BLACK) == 0.5);
    CHECK(zero_one.bilerp_channel({0.5, 0.0}, 0, WrapMode::CLAMP) == 1);
  }

  SECTION("Select") {
    Point2i res(4, 9);
    pstd::vector<float> pix = get_float_pixels(res, 4);
    Image image(pix, res, {"A", "B", "G", "R"});

    auto adesc = image.get_channel_desc({"A"});
    CHECK(bool(adesc));
    Image aimage = image.select_channels(adesc);

    CHECK(aimage.resolution() == image.resolution());
    CHECK(aimage.n_channels() == 1);
    CHECK_THAT(aimage.channel_names(), Equals(std::vector<std::string>{"A"}));
    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        CHECK(aimage.get_channel({x, y}, 0) == image.get_channel({x, y}, 0));
      }
    }

    auto rgdesc = image.get_channel_desc({"R", "G"});
    CHECK(bool(rgdesc));
    Image rgimage = image.select_channels(rgdesc);

    CHECK(rgimage.resolution() == image.resolution());
    CHECK(rgimage.n_channels() == 2);
    CHECK_THAT(rgimage.channel_names(), Equals(std::vector<std::string>{"R", "G"}));
    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        CHECK(rgimage.get_channel({x, y}, 0) == image.get_channel({x, y}, 3));
        CHECK(rgimage.get_channel({x, y}, 1) == image.get_channel({x, y}, 2));
      }
    }
  }
}
