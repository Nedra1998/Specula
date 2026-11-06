#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

#include "specula/util/color.hpp"
#include "specula/util/colorspace.hpp"
#include "specula/util/image.hpp"
#include "specula/util/image/image.hpp"
#include "specula/util/image/image_channel_desc.hpp"
#include "specula/util/image/image_channel_values.hpp"
#include "specula/util/image/image_metadata.hpp"
#include "specula/util/image/pixel_format.hpp"
#include "specula/util/math.hpp"

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

TEST_CASE("ImageIO", "[util][image]") {
  SECTION("PFM") {
    Point2i res(16, 49);
    pstd::vector<float> pixels = get_float_pixels(res, 3);

    Image image(pixels, res, {"R", "G", "B"});
    CHECK(image.write("test.pfm"));

    ImageAndMetadata read = Image::read("test.pfm");
    CHECK(*read.metadata.get_color_space() == *RgbColorSpace::SRGB);
    CHECK(read.image.resolution() == image.resolution());
    CHECK(read.image.format() == PixelFormat::FLOAT);
    CHECK(read.image.n_channels() == 3);

    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        for (int c = 0; c < 3; ++c) {
          CHECK(read.image.get_channel({x, y}, c) == image.get_channel({x, y}, c));
        }
      }
    }

    remove("test.pfm");
  }

  SECTION("EXR") {
    Point2i res(16, 49);
    pstd::vector<float> pixels = get_float_pixels(res, 3);

    for (auto format : {PixelFormat::U256, PixelFormat::HALF, PixelFormat::FLOAT}) {
      Image image(format, res, {"R", "G", "B"}, ColorEncoding::LINEAR);
      image.copy_rect_in({{0, 0}, res}, pixels);

      int offset = 0;
      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          for (int c = 0; c < 3; ++c, ++offset) {
            if (format == PixelFormat::U256) {
            } else if (format == PixelFormat::HALF) {
              REQUIRE(image.get_channel({x, y}, c) == Float(Half(pixels[offset])));
            } else if (format == PixelFormat::FLOAT) {
              REQUIRE(image.get_channel({x, y}, c) == pixels[offset]);
            }
          }
        }
      }

      ImageMetadata metadata;
      metadata.color_space = RgbColorSpace::ACES2065_1;
      CHECK(image.write("test.exr", metadata));

      ImageAndMetadata read = Image::read("test.exr");
      CHECK(*read.metadata.get_color_space() == *RgbColorSpace::ACES2065_1);
      CHECK(read.image.resolution() == image.resolution());
      if (!is_8bit(format)) {
        CHECK(read.image.format() == format);
      }

      ImageChannelDesc desc = read.image.get_channel_desc({"R", "G", "B"});
      REQUIRE(bool(desc));

      for (int y = 0; y < res.y; ++y) {
        for (int x = 0; x < res.x; ++x) {
          ImageChannelValues v = read.image.get_channels({x, y}, desc);
          for (int c = 0; c < 3; ++c) {
            if (format == PixelFormat::U256) {
              CHECK(v[c] == Float(Half(image.get_channel({x, y}, c))));
            } else if (format == PixelFormat::HALF) {
              CHECK(v[c] == Float(Half(image.get_channel({x, y}, c))));
            } else if (format == PixelFormat::FLOAT) {
              CHECK(v[c] == image.get_channel({x, y}, c));
            }
          }
        }
      }

      remove("test.exr");
    }
  }

  SECTION("EXR no metadata") {
    Point2i res(16, 32);
    pstd::vector<float> pixels = get_float_pixels(res, 3);
    Image image(pixels, res, {"R", "G", "B"});

    ImageMetadata metadata;
    metadata.color_space = RgbColorSpace::ACES2065_1;
    CHECK(image.write("nometadata.exr", metadata));

    ImageAndMetadata read = Image::read("nometadata.exr");
    CHECK(*read.metadata.get_color_space() == *RgbColorSpace::ACES2065_1);

    CHECK_FALSE((bool)read.metadata.render_time_seconds);
    CHECK_FALSE((bool)read.metadata.camera_from_world);
    CHECK_FALSE((bool)read.metadata.ndc_from_world);
    CHECK((bool)read.metadata.pixel_bounds);
    CHECK(*read.metadata.pixel_bounds == Bounds2i({0, 0}, res));
    CHECK((bool)read.metadata.full_resolution);
    CHECK(*read.metadata.full_resolution == res);
    CHECK(read.metadata.strings.empty());
    CHECK(read.metadata.string_vectors.empty());

    remove("nometadata.exr");
  }

  SECTION("EXR with metadata") {
    Point2i res(16, 32);
    pstd::vector<float> pixels = get_float_pixels(res, 3);
    Image image(pixels, res, {"R", "G", "B"});

    ImageMetadata metadata;
    metadata.color_space = RgbColorSpace::REC_2020;
    metadata.render_time_seconds = 1234;
    SquareMatrix<4> w2c(3, 1, 4, 1, 5, 9, 2, PI, 2, 7, 1, 8, 2, 8, 1, std::numbers::e_v<float>);
    SquareMatrix<4> w2n(1.5, 2.5, 3.5, 4.75, 5.333, 6.2135, -351.2, -552., 63.2, 47.2, PI,
                        std::cos(1.f), 0, -14, 6, 1e-10f);
    Bounds2i pb({2, 10}, {18, 42});
    Point2i full_res(1000, 200);
    std::map<std::string, std::vector<std::string>> string_vectors;
    string_vectors["foo"] = {"bar", "baz"};

    metadata.camera_from_world = w2c;
    metadata.ndc_from_world = w2n;
    metadata.pixel_bounds = pb;
    metadata.full_resolution = full_res;
    metadata.strings["specula"] = "v1";
    metadata.string_vectors = string_vectors;
    CHECK(image.write("metadata.exr", metadata));

    ImageAndMetadata read = Image::read("metadata.exr");

    CHECK(*read.metadata.get_color_space() == *RgbColorSpace::REC_2020);
    CHECK((bool)read.metadata.render_time_seconds);
    CHECK(*read.metadata.render_time_seconds == 1234);

    CHECK((bool)read.metadata.camera_from_world);
    CHECK(*read.metadata.camera_from_world == w2c);

    CHECK((bool)read.metadata.ndc_from_world);
    CHECK(*read.metadata.ndc_from_world == w2n);

    CHECK((bool)read.metadata.pixel_bounds);
    CHECK(*read.metadata.pixel_bounds == pb);
    CHECK((bool)read.metadata.full_resolution);
    CHECK(*read.metadata.full_resolution == full_res);
    CHECK(read.metadata.strings.size() == 1);
    CHECK(read.metadata.strings["specula"] == "v1");

    CHECK(read.metadata.string_vectors.size() == 1);
    CHECK_THAT(read.metadata.string_vectors["foo"], Equals(std::vector<std::string>{"bar", "baz"}));

    remove("metadata.exr");
  }

  SECTION("PNG Y") {
    Point2i res(11, 50);
    pstd::vector<uint8_t> pixels = get_u8_pixels(res, 1);

    Image image(pixels, res, {"Y"}, ColorEncoding::SRGB);
    CHECK(image.write("test.png"));

    ImageAndMetadata read = Image::read("test.png");
    CHECK(read.image.resolution() == image.resolution());
    CHECK(read.image.format() == PixelFormat::U256);
    CHECK(read.image.n_channels() == 1);
    REQUIRE(read.image.encoding() != nullptr);

    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        CHECK(read.image.get_channel({x, y}, 0) == srgb8_to_linear(pixels[y * res.x + x]));
      }
    }

    remove("test.png");
  }

  SECTION("PNG RGB") {
    Point2i res(11, 50);
    pstd::vector<uint8_t> pixels = get_u8_pixels(res, 3);

    Image image(pixels, res, {"R", "G", "B"}, ColorEncoding::SRGB);
    CHECK(image.write("test.png"));

    ImageAndMetadata read = Image::read("test.png");
    CHECK(read.image.resolution() == image.resolution());
    CHECK(read.image.format() == PixelFormat::U256);
    CHECK(read.image.n_channels() == 3);
    REQUIRE(read.image.encoding() != nullptr);
    CHECK(*read.metadata.get_color_space() == *RgbColorSpace::SRGB);

    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        for (int c = 0; c < 3; ++c) {
          CHECK(read.image.get_channel({x, y}, c) ==
                srgb8_to_linear(pixels[c + 3 * (y * res.x + x)]));
        }
      }
    }

    remove("test.png");
  }

  SECTION("PNG Emoji") {
    Point2i res(11, 50);
    pstd::vector<uint8_t> pixels = get_u8_pixels(res, 3);

    Image image(pixels, res, {"R", "G", "B"}, ColorEncoding::SRGB);
    const uint8_t fn[] = {0xF0, 0x9F, 0xA6, 0x96, '.', 'p', 'n', 'g', '\0'};
    std::string filename(reinterpret_cast<const char *>(fn),
                         strlen(reinterpret_cast<const char *>(fn)));
    CHECK(image.write(filename));

    ImageAndMetadata read = Image::read(filename);
    CHECK(read.image.resolution() == image.resolution());
    CHECK(read.image.format() == PixelFormat::U256);
    CHECK(read.image.n_channels() == 3);
    REQUIRE(read.image.encoding() != nullptr);
    CHECK(*read.metadata.get_color_space() == *RgbColorSpace::SRGB);

    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        for (int c = 0; c < 3; ++c) {
          CHECK(read.image.get_channel({x, y}, c) ==
                srgb8_to_linear(pixels[c + 3 * (y * res.x + x)]));
        }
      }
    }

    remove(filename.c_str());
  }

  SECTION("QOI RGB") {
    Point2i res(11, 50);
    pstd::vector<uint8_t> pixels = get_u8_pixels(res, 3);

    Image image(pixels, res, {"R", "G", "B"}, ColorEncoding::SRGB);
    CHECK(image.write("test.qoi"));

    ImageAndMetadata read = Image::read("test.qoi");
    CHECK(read.image.resolution() == image.resolution());
    CHECK(read.image.format() == PixelFormat::U256);
    CHECK(read.image.n_channels() == 3);
    REQUIRE(read.image.encoding() != nullptr);
    CHECK(*read.metadata.get_color_space() == *RgbColorSpace::SRGB);

    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        for (int c = 0; c < 3; ++c) {
          CHECK(read.image.get_channel({x, y}, c) ==
                srgb8_to_linear(pixels[c + 3 * (y * res.x + x)]));
        }
      }
    }

    remove("test.qoi");
  }

  SECTION("QOI ARGB") {
    Point2i res(11, 50);
    pstd::vector<uint8_t> pixels = get_u8_pixels(res, 4);

    Image image(pixels, res, {"A", "R", "G", "B"}, ColorEncoding::SRGB);
    CHECK(image.write("test-rgba.qoi"));

    ImageAndMetadata read = Image::read("test-rgba.qoi");
    CHECK(read.image.resolution() == image.resolution());
    CHECK(read.image.format() == PixelFormat::U256);
    CHECK(read.image.n_channels() == 4);
    REQUIRE(read.image.encoding() != nullptr);
    CHECK(*read.metadata.get_color_space() == *RgbColorSpace::SRGB);

    ImageChannelDesc desc = read.image.get_channel_desc({"A", "R", "G", "B"});
    REQUIRE((bool)desc);

    for (int y = 0; y < res.y; ++y) {
      for (int x = 0; x < res.x; ++x) {
        ImageChannelValues v = read.image.get_channels({x, y}, desc);
        for (int c = 0; c < 4; ++c) {
          CHECK(v[c] == srgb8_to_linear(pixels[c + 4 * (y * res.x + x)]));
        }
      }
    }

    remove("test-rgba.qoi");
  }

  SECTION("Round trip EXR") { test_round_trip("out.exr"); }
  SECTION("Round trip PFM") { test_round_trip("out.pfm"); }
  SECTION("Round trip PNG") { test_round_trip("out.png"); }
  SECTION("Round trip QOI") { test_round_trip("out.qoi"); }
}
